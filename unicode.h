

// Specifies how to interpret characters 0x00-0x1f and 0x7f in DOS strings
enum class DosStringConvertMode {

	// String contains control codes (new line, tabulation, delete, etc.)
	WithControlCodes,

	// String does not have any codes characters, consider all characters
	// as screen codes
	ScreenCodesOnly,

	// String should not contain characters mentioned above
	NoSpecialCharacters
}
