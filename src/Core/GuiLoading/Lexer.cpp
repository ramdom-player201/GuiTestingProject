#include "Lexer.h"

#include <cctype>
#include <format>

namespace {
	bool IsIdentStart(char c) { // An identity starts if it is an alphabetical character or '_' but not if numeric
		return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
	}

	bool IsIdentContinue(char c) { // An identity can contain any alphanumerical character plus '_'
		return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
	}

	bool IsDigit(char c) { // Only numbers
		return std::isdigit(static_cast<unsigned char>(c));
	}

	bool IsHexDigit(char c) { // Numbers plus A-F
		return std::isxdigit(static_cast<unsigned char>(c));
	}
}

Lexer::Lexer(std::string_view src) : source(src) {
	LogService::Log(LogType::TRACE, className, "Constructor", "Lexer created");
}

bool Lexer::IsEndReached() const {
	return pos >= source.size();
}

char Lexer::PeekChar(size_t ahead) const {
	if (pos + ahead >= source.size()) { return '\0'; } // Returns null-terminator if past bounds
	return source[pos + ahead]; // Return character at requested position if within bounds
}

char Lexer::AdvanceChar() {
	if (IsEndReached()) { return '\0'; } // Cannot advance past end, return null-terminator
	char c{ source[pos++] }; // Get current character and advance
	if (c == '\n') { // If newline, increment line and reset column
		line++;
		col = 1;
	}
	else { // Else, increment column
		col++;
	}
	return c; // Return current character
}

void Lexer::SkipWhitespaceAndComments() {
	while (!IsEndReached()) {
		char c{ PeekChar() }; // Get char at current pos
		if (c == ' ' || c == '\t' || c == '\r') {
			AdvanceChar(); // Skip for each whitespace
		}
		else if (c == '@') {
			// Comment, need to jump to next line
			while (!IsEndReached() && PeekChar() != '\n') {
				AdvanceChar(); // Continually advance until newline is reached without consuming newline
			}
		}
		else {
			break; // Current char is no longer whitespace
		}
	}
}

Token Lexer::ScanIdent() {
	// Store starting point of the Identity token
	size_t startLine{ line };
	size_t startCol{ col };
	size_t startPos{ pos };

	// Special case, singular '_' underscore represents a NULL/DEFAULT/NO-OP value :: Underscore token type
	if (source[startPos] == '_' && !IsIdentContinue(PeekChar(1))) {
		// If next character is not valid indent character
		AdvanceChar(); // Consume underscore
		return Token{ TokenType::Underscore, source.substr(startPos, 1), startLine, startCol };
	}

	// Assuming not a singular underscore, find end of identifier
	while (!IsEndReached() && IsIdentContinue(PeekChar())) {
		AdvanceChar();
	}

	return Token{ TokenType::Ident, source.substr(startPos, pos - startPos), startLine, startCol };
}

Token Lexer::ScanNumber() {
	// Store starting point of the Numeric token
	size_t startLine{ line };
	size_t startCol{ col };
	size_t startPos{ pos };

	// Numbers can start with '-' to symbolise negative or (optionally) '+' for positive
	if (PeekChar() == '-' || PeekChar() == '+') {
		AdvanceChar();
	}

	// Parse first part of number
	while (!IsEndReached() && IsDigit(PeekChar())) {
		AdvanceChar();
	}

	// Numbers can be split up to one time with '.' for decimals, parse optional second half of number
	if (!IsEndReached() && PeekChar() == '.') {
		AdvanceChar();
		while (!IsEndReached() && IsDigit(PeekChar())) {
			AdvanceChar();
		}
	}

	return Token{ TokenType::Number, source.substr(startPos, pos - startPos), startLine, startCol };
}

Token Lexer::ScanHex() {
	size_t startLine{ line };
	size_t startCol{ col };
	size_t startPos{ pos };

	AdvanceChar(); // Consume the '#' that marks the start of a hex

	// Parse hex value
	while (!IsEndReached() && IsHexDigit(PeekChar())) {
		AdvanceChar();
	}

	// Error checking only checks for missing digits, not for other invalid hex configurations
	if (pos == startPos + 1) {
		// No hex digits were found after the '#' signifier
		LogService::Log(
			LogType::ERROR, className, "ScanHex",
			std::format("Malformed hex literal at line {}, col {} (no digits after #)", startLine, startCol)
		);

	}

	return Token{ TokenType::Hex, source.substr(startPos, pos - startPos), startLine, startCol };
}

Token Lexer::ScanString() {
	size_t startLine{ line };
	size_t startCol{ col };
	size_t startPos{ pos };

	AdvanceChar(); // Consume opening quotation mark

	// Parse string to closing quotation mark, or end of line
	while (!IsEndReached() && PeekChar() != '"' && PeekChar() != '\n') {
		AdvanceChar();
	}

	// Handle unterminated string error
	if (IsEndReached() || PeekChar() == '\n') {
		LogService::Log(
			LogType::ERROR, className, "ScanString",
			std::format("Unterminated string at line {}, col {}", startLine, startCol)
		);
		// Return excluding first quotation mark, second does not exist to exclude
		return Token{ TokenType::String, source.substr(startPos + 1, pos - startPos - 1), startLine, startCol };
	}

	AdvanceChar(); // Consume closing quote
	// Return excluding first and last quotation marks
	return Token{ TokenType::String, source.substr(startPos + 1, pos - startPos - 2), startLine, startCol };
}

Token Lexer::NextToken() {
	SkipWhitespaceAndComments();

	// Return if end of file
	if (IsEndReached()) {
		return Token{ TokenType::Eof,{}, line, col };
	}

	size_t startLine{ line };
	size_t startCol{ col };
	char c{ PeekChar() };

	// Newline
	if (c == '\n') {
		AdvanceChar();
		return Token{ TokenType::Newline,{}, startLine, startCol };
	}

	// Identifier or underscore
	if (IsIdentStart(c)) {
		return ScanIdent();
	}

	// Number (optional leading +/-)
	if (IsDigit(c) || ((c == '-' || c == '+') && !IsEndReached() && IsDigit(PeekChar(1)))) {
		return ScanNumber();
	}

	// Hex
	if (c == '#') {
		return ScanHex();
	}

	// String
	if (c == '"') {
		return ScanString();
	}

	// Single character tokens plus [<-]
	AdvanceChar(); // Consume token
	std::string_view lex{ source.substr(pos - 1, 1) };

	switch (c) {
	case '{': return Token{ TokenType::BraceL, lex, startLine, startCol };
	case '}': return Token{ TokenType::BraceR, lex, startLine, startCol };
	case ':': return Token{ TokenType::Colon, lex, startLine, startCol };
	case '=': return Token{ TokenType::Assign, lex, startLine, startCol };
	case '.': return Token{ TokenType::Dot, lex, startLine, startCol };
	case '>': return Token{ TokenType::AngularR, lex, startLine, startCol };
	case '<': // either AnglularL or Arrow
	{
		if (!IsEndReached() && PeekChar() == '-') { // If second is '-' then token is Arrow
			AdvanceChar();
			return Token{ TokenType::Arrow, source.substr(pos - 2, 2), startLine, startCol };
		}
		return Token{ TokenType::AngularL, lex, startLine, startCol };
	}
	default:
		// Unknown character not in DSL, error and skip
		LogService::Log(
			LogType::ERROR, className, "ScanToken",
			std::format(
				"Unknown character '{}' (0x{:02X}) at line {}, col {}",
				c, static_cast<unsigned char>(c), startLine, startCol
			)
		);
		// Lexer has already advanced, so we don't need to skip again
		return NextToken(); // Recursively scan to ensure a valid token is returned
	}
}