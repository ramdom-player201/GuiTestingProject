#pragma once

#include "../Services/LogService.h"

enum class TokenType : uint8_t {
	Ident,		// Identity, string that represents a type/variable/property
	Number,		// Numeric value, no separation between int/float
	Hex,		// Short or long hex code, only really supported for colour values
	String,		// String, primarily used for filepaths but could be used as a value for a textual property
	BraceL,		// Start a struct block
	BraceR,		// End a struct block
	Colon,		//
	Assign,		// Used in property=value assignments
	Dot,		// Used to access sub-components
	Arrow,		// Insertion operator used in Build block to reparent elements
	AngularL,	// Start a dynamic block
	AngularR,	// End a dynamic block
	Underscore,	// Represents a null/no-op/default value
	//At,		// Comment indicator
	Newline,	// Line terminator
	Eof,		// End of file
};

struct Token {
	TokenType type{ TokenType::Eof };
	std::string_view lexeme; // Points into source string
	size_t line{ 0 };
	size_t col{ 0 };
};

// Class definition
class Lexer {
private:
	std::string_view source;
	size_t pos{ 0 };
	size_t line{ 1 };
	size_t col{ 1 };

	// Internal scanning
	bool IsEndReached() const;
	char PeekChar(size_t ahead = 0) const;
	char AdvanceChar();
	void SkipWhitespaceAndComments();

	// Sub-scanners
	Token ScanIdent();
	Token ScanNumber();
	Token ScanHex();
	Token ScanString();

	//Token ScanToken(); // Primary scan function, can call sub-scanners

public:
	explicit Lexer(std::string_view source);
	~Lexer() = default;

	// Safety locks
	Lexer() = delete;
	Lexer(const Lexer&) = delete;
	Lexer& operator=(const Lexer&) = delete;
	Lexer(Lexer&&) = delete;
	Lexer& operator=(Lexer&&) = delete;

	Token NextToken();

	size_t GetCurrentLine() const { return line; }
	size_t GetCurrentCol() const { return col; }

	// ClassName
	static constexpr std::string_view className{ "Lexer" };
};