/*
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 *  Copyright (C) 2024-2024  The DOSBox Staging Team
 */

// Specifies how to interpret characters 0x00-0x1f and 0x7f in DOS strings
enum class DosStringConvertMode {
	WithControlCodes, // String contains control codes (new line, tabulation, delete, etc.)
	ScreenCodesOnly, // String does not have any codes characters, consider all characters as screen codes
	NoSpecialCharacters, // String should not contain characters mentioned above
}
