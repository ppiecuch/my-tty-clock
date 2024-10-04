#include <cstdlib>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "par_easycurl.h"

const std::map<std::string, std::string> lang_codes{
	{ "af", "Afrikaans" },
	{ "sq", "Albanian" },
	{ "ar", "Arabic" },
	{ "hy", "Armenian" },
	{ "az", "Azerbaijani" },
	{ "eu", "Basque" },
	{ "be", "Belarusian" },
	{ "bn", "Bengali" },
	{ "bs", "Bosnian" },
	{ "km", "Cambodian" },
	{ "ca", "Catalan" },
	{ "zh-CN", "Chinese (Simplified)" },
	{ "zh-TW", "Chinese (Traditional)" },
	{ "hr", "Croatian" },
	{ "cs", "Czech" },
	{ "da", "Danish" },
	{ "nl", "Dutch" },
	{ "en", "English" },
	{ "eo", "Esperanto" },
	{ "et", "Estonian" },
	{ "tl", "Filipino" },
	{ "fi", "Finnish" },
	{ "fr", "French" },
	{ "ka", "Georgian" },
	{ "de", "German" },
	{ "el", "Greek" },
	{ "gu", "Gujarati" },
	{ "hi", "Hindi" },
	{ "hu", "Hungarian" },
	{ "is", "Icelandic" },
	{ "id", "Indonesian" },
	{ "it", "Italian" },
	{ "ja", "Japanese" },
	{ "jw", "Javanese" },
	{ "kn", "Kannada" },
	{ "ko", "Korean" },
	{ "la", "Latin" },
	{ "lv", "Latvian" },
	{ "mk", "Macedonian" },
	{ "ml", "Malayalam" },
	{ "mr", "Marathi" },
	{ "mo", "Moldavian" },
	{ "sr-ME", "Montenegrin" },
	{ "ne", "Nepali" },
	{ "no", "Norwegian" },
	{ "pl", "Polish" },
	{ "pt-BR", "Portuguese (Brazil)" },
	{ "pt-PT", "Portuguese (Portugal)" },
	{ "ro", "Romanian" },
	{ "ru", "Russian" },
	{ "sr", "Serbian" },
	{ "sh", "Serbo-Croatian" },
	{ "si", "Sinhalese" },
	{ "sk", "Slovak" },
	{ "sl", "Slovenian" },
	{ "es", "Spanish" },
	{ "es-419", "Spanish (Latin American)" },
	{ "su", "Sundanese" },
	{ "sw", "Swahili" },
	{ "sv", "Swedish" },
	{ "ta", "Tamil" },
	{ "te", "Telugu" },
	{ "th", "Thai" },
	{ "tr", "Turkish" },
	{ "uk", "Ukrainian" },
	{ "ur", "Urdu" },
	{ "vi", "Vietnamese" },
	{ "cy", "Welsh" },
};

class GoogleTTS {
	std::string _curl = "curl 'https://translate.google.com/translate_tts?ie=UTF-8&q=";
	std::string _lang = "&tl=";
	std::string _text = "";
	std::string _client = "&client=tw-ob' ";
	std::string _out = "> /tmp/gtts.mp3";
	std::string _outv = "> /tmp/gtts_";
	std::string _ref = " 'Referer: http://translate.google.com/' ";
	std::string _agent = " 'User-Agent: stagefright/1.2 (Linux;Android 9.0)' ";
	std::string _mpv = "mpg321";
	std::string _speed = " --speed=";
	std::string _play = " /tmp/gtts.mp3 1>/dev/null";
	std::string _cat = "cat /tmp/gtts_*.mp3 > /tmp/gtts.mp3";
	std::string _rm = "rm /tmp/gtts_*.mp3";

	std::vector<std::string> _cmds;

	void parse(std::vector<std::string> &vec);
	void parse();
	std::vector<std::string> split(std::string &msg);
	void replace(std::string &text);
	void unite();

public:
	GoogleTTS(std::string msg, std::string lang, std::string speed = "1.0");
	void execute();
	static void help();
	static void version();
	static void languages();
};

int main(int argc, char *argv[]) {
	switch (argc) {
		case 2:
			if (std::string("-h") == argv[1] || std::string("-H") == argv[1] || std::string("--help") == argv[1]) {
				GoogleTTS::help();
			}
			if (std::string("-v") == argv[1] || std::string("--version") == argv[1]) {
				GoogleTTS::version();
			}
			if (std::string("-l") == argv[1] || std::string("--languages") == argv[1]) {
				GoogleTTS::languages();
			}
			break;

		case 3: {
			std::string msg(argv[2]);
			std::string lang(argv[1]);
			std::unique_ptr<GoogleTTS> tts(new GoogleTTS(msg, lang));
			tts->execute();
		} break;

		case 4: {
			std::string msg(argv[2]);
			std::string lang(argv[1]);
			std::string speed(argv[3]);
			std::unique_ptr<GoogleTTS> tts(new GoogleTTS(msg, lang, speed));
			tts->execute();
		} break;

		default:
			GoogleTTS::help();
			break;
	}
	return 0;
}

/// GTTS

GoogleTTS::GoogleTTS(std::string msg, std::string lang, std::string speed) {
	_speed += speed;
	_lang += lang;
	if (msg.length() > 200) {
		std::vector<std::string> msgs = this->split(msg);
		this->parse(msgs);
	} else {
		_text = msg;
		this->parse();
	}
}

std::vector<std::string> GoogleTTS::split(std::string &msg) {
	std::vector<std::string> vec;
	std::istringstream iss(msg);
	std::vector<std::string> words(std::istream_iterator<std::string>{ iss }, std::istream_iterator<std::string>());
	std::string part = "";
	for (std::string s : words) {
		if (part.size() - 1 + s.size() <= 200) {
			part += s + " ";
		} else {
			vec.push_back(part);
			part = s + " ";
		}
	}
	if (part != "")
		vec.push_back(part);

	return vec;
}

void GoogleTTS::unite() {
	system(_cat.c_str());
	system(_rm.c_str());
}

void GoogleTTS::execute() {
	if (_cmds.size() == 1) {
		std::system(_cmds[0].c_str());
	} else {
		for (std::string cmd : _cmds) {
			std::system(cmd.c_str());
		}
		this->unite();
	}
	_mpv += _speed + _play;
	std::system(_mpv.c_str());
}

void GoogleTTS::replace(std::string &text) {
	size_t start_pos = 0;
	while ((start_pos = text.find(" ", start_pos)) != std::string::npos) {
		text.replace(start_pos, 1, "%20");
		start_pos += 3; // Handles case where 'to' is a substring of 'from'
	}
}

void GoogleTTS::parse() {
	this->replace(_text);
	std::string cmd = _curl + _text + _lang + _client + "-H" + _ref + "-H";
	cmd += _agent + _out + " 2>/dev/null";
	_cmds.push_back(cmd);
}

void GoogleTTS::parse(std::vector<std::string> &vec) {
	std::string cmd = "";
	int i = 0;
	for (std::string msg : vec) {
		this->replace(msg);
		cmd = _curl + msg + _lang + _client + "-H" + _ref + "-H";
		cmd += _agent + _outv + std::to_string(i) + ".mp3" + " 2>/dev/null";
		_cmds.push_back(cmd);
		i++;
	}
}

void GoogleTTS::help() {
	std::cout << "gtts: plays Google Text-to-Speech speech synthesis with " << "help of Google Translate voice" << std::endl;
	std::cout << "Usage: gtts [language] \"[message]\" ([speed])" << std::endl;
	std::cout << "Example: gtts us \"hello world\" 1.5" << std::endl;
	std::cout << "standard speed is 1.0" << std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "-h\t\tshows this help" << std::endl;
	std::cout << "-v\t\tshows program version" << std::endl;
	std::cout << "-l\t\tshows all available languages" << std::endl;
	std::cout << std::endl;
	std::cout << "To speak from text files use: gtts [lang] \"$(cat file.txt)\" ([speed])" << std::endl;
	std::cout << std::endl;
}

void GoogleTTS::languages() {
	std::cout << "Supported languages:" << std::endl;
	int counter = 0;
	for (std::pair<std::string, std::string> l : lang_codes) {
		std::cout << l.first << "\t:\t" << l.second << std::endl;
	}
	std::cout << std::endl;
}

void GoogleTTS::version() { std::cout << "gtts version: 0.3" << std::endl; }
