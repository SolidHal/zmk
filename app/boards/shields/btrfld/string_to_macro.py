#!/bin/python3.10









test_macro_string = """I am a really cool
test1
String
"""

char_to_kc = {
    " "  : "SPC",
    "\n": "ENTER",
    "(" : "LEFT_PARENTHESIS",
    ")" : "RIGHT_PARENTHESIS",
    "[" : "LEFT_BRACKET",
    "]" : "RIGHT_BRACKET",
    "{" : "LEFT_BRACE",
    "}" : "RIGHT_BRACE",
    ":" : "COLON",
    ";" : "SEMICOLON",
    "'" : "SINGLE_QUOTE",
    '"' : "DOUBLE_QUOTE",
    "<" : "LESS_THAN",
    ">" : "GRATER_THAN",
    "/" : "SLASH",
    "\\" : "BACKSLASH",
    "|" : "PIPE",
    "*" : "ASTERISK",
    "%" : "PERCENT",
    "&" : "AMPERSAND",
    "$" : "DOLLAR",
    "#" : "HASH",
    "@" : "AT_SIGN",
    "!" : "EXCLAMATION",
    "." : "DOT",
}

def convert(input_string):
    for char in input_string:
        if char.isupper():
            print(f", <&macro_press &kp LSHFT>")
            print(f", <&macro_tap &kp {char}>")
            print(f", <&macro_release &kp LSHFT>")

        elif char.isalpha():
            print(f", <&macro_tap &kp {char.upper()}>")

        elif char in char_to_kc.keys():
            print(f", <&macro_tap &kp {char_to_kc[char]}>")

        elif char.isnumeric():
            print(f", <&macro_tap &kp N{char}>")
        else:
            print(f"TODO: not implemented. cannot handle:{char}")

convert(test_macro_string)
print("")

print("")