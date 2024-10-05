#ifndef GTTS_H
#define GTTS_H

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

const std::string _curl = "curl ";
const std::string _tts = "'https://translate.google.com/translate_tts?ie=UTF-8&q=";
const std::string _lang = "&tl=";
const std::string _text = "";
const std::string _client = "&client=tw-ob' ";
const std::string _out = "> /tmp/gtts.mp3";
const std::string _outv = "> /tmp/gtts_";
const std::string _ref = " 'Referer: http://translate.google.com/' ";
const std::string _agent = " 'User-Agent: stagefright/1.2 (Linux;Android 9.0)' ";
const std::string _mpv = "mpg321";
const std::string _speed = " --speed=";
const std::string _play = " /tmp/gtts.mp3 1>/dev/null";
const std::string _cat = "cat /tmp/gtts_*.mp3 > /tmp/gtts.mp3";
const std::string _rm = "rm /tmp/gtts_*.mp3";

class GoogleTTS {
	std::vector<std::string> _cmds;

	void parse(std::vector<std::string> &vec);
	void parse();
	std::vector<std::string> split(std::string &msg);
	void replace(std::string &text);
	void unite();

	static bool verbose = false;

public:
	static void help();
	static void version();
	static void languages();

	static void enable_verbose() { verbose = true; }
	static void disable_verbose() { verbose = false; }

	void execute();
	GoogleTTS(std::string msg, std::string lang, std::string speed = "1.0");
};

#endif // GTTS_H
