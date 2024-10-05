#ifndef GTTS_H
#define GTTS_H

#include <map>
#include <string>
#include <vector>

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

const std::string _CURL = "curl ";
const std::string _TTS = "'https://translate.google.com/translate_tts?ie=UTF-8&q=";
const std::string _LANG = "&tl=";
const std::string _CLIENT = "&client=tw-ob' ";
const std::string _OUT = "> /tmp/gtts.mp3";
const std::string _OUTV = "> /tmp/gtts_";
const std::string _REF = " 'Referer: http://translate.google.com/' ";
const std::string _AGENT = " 'User-Agent: stagefright/1.2 (Linux;Android 9.0)' ";
const std::string _MPV = "mpg321";
const std::string _SPEED = " --speed=";
const std::string _PLAY = " /tmp/gtts.mp3 1>/dev/null";
const std::string _CAT = "cat /tmp/gtts_*.mp3 > /tmp/gtts.mp3";
const std::string _RM = "rm /tmp/gtts_*.mp3";

class GoogleTTS {
	std::vector<std::string> _cmds;

	void parse(std::vector<std::string> &vec);
	void parse();
	std::vector<std::string> split(std::string &msg);
	void replace(std::string &text);
	void unite();

	bool verbose = false;

	std::string _text = "";
	std::string _speed = _SPEED;
	std::string _lang = _LANG;

public:
	static void help();
	static void version();
	static void languages();

	void setup_verbose(bool v) { verbose = v; }

	void execute();
	GoogleTTS(std::string msg, std::string lang, std::string speed = "1.0");
};

#endif // GTTS_H
