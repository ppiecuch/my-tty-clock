/*
 * Copyright (c) 2014, Enrico Bertolazzi (enrico.bertolazzi@unitn.it)
 * All rights reserved.
 */

#include "figlet_font.h"
#include <string.h>

#ifdef EMBED_FIGLET_USE_VISUAL_STUDIO
#define STRCPY(TO, FROM, MAXLEN) strcpy_s(TO, MAXLEN, FROM);
#else
#define STRCPY(TO, FROM, MAXLEN) strncpy(TO, FROM, MAXLEN);
#endif

namespace Figlet {

typedef unsigned short u_short;

Banner::Banner(
		FontFiglet const *_characters,
		char _Hardblank,
		unsigned _Height,
		unsigned _FontMaxLen,
		unsigned _FontSize,
		PrintMode _PrintMode,
		const char *_MappingFrom,
		const char *_MappingTo) :
		characters(_characters), Hardblank(_Hardblank), Height(_Height), FontMaxLen(_FontMaxLen), FontSize(_FontSize), charPosition(0), mappingFrom(_MappingFrom), mappingTo(_MappingTo), printMode(_PrintMode) {
	std::fill(charToTable, charToTable + maxTableSize, 0); // caratteri non noti mappati in spazi
	std::fill(rspaces, rspaces + Height, 0);
	for (unsigned i = 0; i < FontSize; ++i) {
		unsigned ipos = unsigned(characters[i].nchar);
		if (ipos < maxTableSize) {
			charToTable[ipos] = u_short(i);
			charWidth[ipos] = u_short(strlen(characters[i].rows[0]));
		}
	}
	Width = charWidth[charToTable[int('M')]];
}

void Banner::init() {
	charPosition = 0;
	std::fill(rspaces, rspaces + Height, 0);
	for (unsigned i = 0; i < Height; ++i)
		*lines[i] = '\0';
}

// ---------------------------------------------------------------------------

bool Banner::pushFullWidth(unsigned c) {
	FontFiglet const *f = characters + charToTable[c];
	unsigned cw = charWidth[c];

	// controllo che il carattere stia nel buffer
	if (charPosition + cw > maxLenght)
		return false;

	unsigned maxlen = maxLenght - charPosition;
	for (unsigned i = 0; i < Height; ++i)
		STRCPY(lines[i] + charPosition, f->rows[i], maxlen);

	charPosition += cw;

	// aggiorno spazi liberi a destra
	std::copy(f->rspaces, f->rspaces + Height, rspaces);
	return true;
}

// ---------------------------------------------------------------------------

bool Banner::pushMonospaced(unsigned c) {
	FontFiglet const *f = characters + charToTable[c];
	unsigned cw = charWidth[c];
	unsigned dd = Width >= cw ? Width - cw : 0; // extra spazi

	// controllo che il carattere stia nel buffer
	if (charPosition + cw + dd > maxLenght)
		return false;

	// centratura carattere
	unsigned dR = dd / 2;
	unsigned dL = dd - dR;

	for (unsigned i = 0; i < Height; ++i) {
		char *p = lines[i] + charPosition;
		char const *q = f->rows[i];
		for (unsigned j = 0; j < dL; ++j)
			*p++ = ' ';
		for (unsigned j = 0; j < cw; ++j)
			*p++ = *q++;
		for (unsigned j = 0; j < dR; ++j)
			*p++ = ' ';
		*p = '\0';
	}
	charPosition += cw + dd;

	// aggiorno spazi liberi a destra
	std::copy(f->rspaces, f->rspaces + Height, rspaces);
	return true;
}

// ---------------------------------------------------------------------------

bool Banner::pushPacked(unsigned c) {
	FontFiglet const *f = characters + charToTable[c];
	unsigned cw = unsigned(charWidth[c]);
	// calcolo overlapping
	unsigned overlap = rspaces[0] + f->lspaces[0];
	for (unsigned i = 1; i < Height; ++i) {
		unsigned tmp = rspaces[i] + f->lspaces[i];
		if (tmp < overlap)
			overlap = tmp;
	}

	// controllo che il carattere stia nel buffer
	if (charPosition + cw > maxLenght + overlap)
		return false;

	for (unsigned i = 0; i < Height; ++i) {
		// copio porzione di stringa
		if (overlap > f->lspaces[i]) {
			unsigned charP = (charPosition + f->lspaces[i]) - overlap;
			unsigned maxlen = maxLenght - charP;
			STRCPY(lines[i] + charP, f->rows[i] + f->lspaces[i], maxlen);
		} else {
			unsigned maxlen = maxLenght - charPosition;
			STRCPY(lines[i] + charPosition, f->rows[i] + overlap, maxlen);
		}
	}
	charPosition += cw - overlap;

	// aggiorno spazi liberi a destra
	std::copy(f->rspaces, f->rspaces + Height, rspaces);
	return true;
}

// ---------------------------------------------------------------------------

/*
  static
  inline
  unsigned
  findClass( char c ) {
	if ( c == '|' ) return 1;
	if ( c == '/' || c == '\\' ) return 3;
	if ( c == '[' || c == ']'  ) return 4;
	if ( c == '{' || c == '}'  ) return 5;
	if ( c == '(' || c == ')'  ) return 6;
	return 0;
  }
  */

char Banner::smushingRules(char left, char right) const {
	// rule 0: left blank use right
	if (left == ' ')
		return right;
	// rule 1: equal character smushing
	if (left == right)
		return right;
	// rule 2: underscore smushing
	if (left == '_' && strchr("|/\\[]{}()<>", right) != nullptr)
		return right;
	if (right == '_' && strchr("|/\\[]{}()<>", left) != nullptr)
		return left;
	// rule 3: hierarchy smushing
	/* ELIMINATED
	unsigned class_left  = findClass( left );
	unsigned class_right = findClass( right );
	if ( class_left > 0 && class_right > 0 ) {
	  if      ( class_left < class_right ) return right;
	  else if ( class_left > class_right ) return left;
	}
	*/
	// rule 4: opposite pair smushing
	if (left == '[' && right == ']')
		return '|';
	if (left == ']' && right == '[')
		return '|';
	if (left == '{' && right == '}')
		return '|';
	if (left == '}' && right == '{')
		return '|';
	if (left == '(' && right == ')')
		return '|';
	if (left == ')' && right == '(')
		return '|';
	// rule 5: big X smushing
	if (left == '/' && right == '\\')
		return '|';
	if (left == '\\' && right == '/')
		return 'Y';
	if (left == '>' && right == '<')
		return 'X';
	// rule 6: hardblack smushing
	// extra rules
	return '\0';

	// not clear it if work well, for the moment are disables
	/*
	if ( left == '<' && right == '|' ) return '<';
	if ( left == '|' && right == '/' ) return '/';
	if ( left == '|' && right == '(' ) return right;
	if ( left == ')' && right == '|' ) return left;
	if ( (left == '\\' || left == '/') && right == '|' ) return left;
	return '\0';
	*/
}

bool Banner::pushSmushed(unsigned c) {
	FontFiglet const *fchar = characters + charToTable[c];
	unsigned cw = charWidth[c];
	// calcolo overlapping
	unsigned overlap = rspaces[0] + fchar->lspaces[0];
	for (unsigned i = 1; i < Height; ++i) {
		unsigned tmp = rspaces[i] + fchar->lspaces[i];
		if (tmp < overlap)
			overlap = tmp;
	}

	// calcolo smush se possibile
	if (charPosition > 0) {
		bool do_smush = true;
		for (unsigned i = 0; i < Height; ++i) {
			unsigned tmp = rspaces[i] + fchar->lspaces[i];
			if (tmp == overlap) {
				char *pline = lines[i] + charPosition - rspaces[i];
				smush[i] = smushingRules(pline[-1], fchar->rows[i][fchar->lspaces[i]]);
				do_smush = do_smush && (smush[i] != '\0');
			}
		}
		if (do_smush)
			++overlap;
	}

	// controllo che il carattere stia nel buffer
	unsigned cwo = cw - overlap; // >= 0
	if (charPosition + cwo > maxLenght)
		return false;

	for (unsigned i = 0; i < Height; ++i) {
		char *pline = lines[i] + charPosition;
		unsigned tmp = rspaces[i] + fchar->lspaces[i];
		unsigned maxlen = maxLenght - charPosition;
		if (tmp < overlap) {
			pline -= rspaces[i] + 1;
			*pline++ = smush[i];
			maxlen += rspaces[i];
			STRCPY(pline, fchar->rows[i] + fchar->lspaces[i] + 1, maxlen);
		} else {
			if (overlap > fchar->lspaces[i]) {
				pline -= (overlap - fchar->lspaces[i]);
				maxlen += (overlap - fchar->lspaces[i]);
				STRCPY(pline, fchar->rows[i] + fchar->lspaces[i], maxlen);
			} else {
				STRCPY(pline, fchar->rows[i] + overlap, maxlen);
			}
		}
	}
	// overlap <= cw;
	charPosition += cwo;

	// aggiorno spazi liberi alla destra ultimo carattere inserito
	for (unsigned i = 0; i < Height; ++i) {
		rspaces[i] += cwo;
		if (rspaces[i] > fchar->rspaces[i])
			rspaces[i] = fchar->rspaces[i];
	}
	//std::copy( f -> rspaces, f -> rspaces + Height, rspaces );
	return true;
}

// ---------------------------------------------------------------------------

void Banner::fillForPrint(char const message[]) {
	init();
	switch (printMode) {
		case FIGLET_SMUSHED:
			for (char const *p = message; *p != '\0'; ++p)
				pushSmushed(unsigned(*p));
			break;
		case FIGLET_PACKED:
			for (char const *p = message; *p != '\0'; ++p)
				pushPacked(unsigned(*p));
			break;
		case FIGLET_FULLWIDTH:
			for (char const *p = message; *p != '\0'; ++p)
				pushFullWidth(unsigned(*p));
			break;
		case FIGLET_MONOSPACED:
			for (char const *p = message; *p != '\0'; ++p)
				pushMonospaced(unsigned(*p));
			break;
	}

	// replace Hardblank
	for (unsigned i = 0; i < Height; ++i) {
		char *p = lines[i];
		do {
			if (*p == Hardblank)
				*p = ' ';
		} while (*p++ != '\0');
	}

	// process character mapping
	if (mappingFrom && mappingTo) {
		for (unsigned i = 0; i < Height; ++i) {
			char *p = lines[i];
			do {
				for (unsigned k = 0; mappingFrom[k] && mappingTo[k]; k++) {
					if (*p == mappingFrom[k])
						*p = mappingTo[k];
				}
			} while (*p++ != '\0');
		}
	}
}

// ---------------------------------------------------------------------------

unsigned
Banner::print(
		char const message[],
		ostream_type &s,
		char const top[],
		char const bottom[]) {
	fillForPrint(message);

	for (unsigned i = 0; i < strlen(top); ++i) {
		for (unsigned j = 0; j < charPosition; ++j)
			s << top[i];
		s << '\n';
	}
	for (unsigned i = 0; i < Height; ++i)
		s << lines[i] << '\n';
	for (unsigned i = 0; i < strlen(bottom); ++i) {
		for (unsigned j = 0; j < charPosition; ++j)
			s << bottom[i];
		s << '\n';
	}
	return charPosition;
}

// ---------------------------------------------------------------------------

void Banner::printFramed(
		char const message[],
		ostream_type &s,
		FrameMode fm) {
	fillForPrint(message);

	switch (fm) {
		case FIGLET_SINGLE:
			s << '+';
			for (unsigned j = 0; j < charPosition + 2; ++j)
				s << '-';
			s << "+\n";
			for (unsigned i = 0; i < Height; ++i)
				s << "| " << lines[i] << " |\n";
			s << '+';
			for (unsigned j = 0; j < charPosition + 2; ++j)
				s << '-';
			s << "+\n";
			break;
		case FIGLET_DOUBLE:
			s << '@';
			for (unsigned j = 0; j < charPosition + 2; ++j)
				s << '=';
			s << "@\n";
			for (unsigned i = 0; i < Height; ++i)
				s << "# " << lines[i] << " #\n";
			s << '@';
			for (unsigned j = 0; j < charPosition + 2; ++j)
				s << '=';
			s << "@\n";
			break;
	}
}

} // namespace Figlet
