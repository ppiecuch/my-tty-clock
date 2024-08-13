/*
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 *  Copyright (C) 2024-2024  The DOSBox Staging Team
 */

#include <cstdint>
#include <string>

#include "unicode.h"

static std::map<uint16_t, code_page_maps_t> per_code_page_mappings = {};

static wide_string dos_to_wide(const std::string& str,
                               const DosStringConvertMode convert_mode,
                               const uint16_t code_page)
{
	wide_string str_out = {};
	str_out.reserve(str.size());

	for (const auto character : str) {
		const auto byte = static_cast<uint8_t>(character);
		if (byte >= DecodeThresholdNonAscii) {
			// Take from code page mapping
			auto& mappings = per_code_page_mappings[code_page];

			if ((per_code_page_mappings.count(code_page) == 0) ||
			    (mappings.grapheme_to_dos.count(byte) == 0)) {
				str_out.push_back(UnknownCharacter);
			} else {
				mappings.grapheme_to_dos[byte].PushInto(str_out);
			}
		} else if (is_control_code(byte)) {
			const auto wide = screen_code_to_wide(byte, convert_mode);
			if (wide) {
				str_out.push_back(*wide);
			} else {
				str_out.push_back(UnknownCharacter);
			}
		} else {
			str_out.push_back(byte);
		}
	}

	return str_out;
}

