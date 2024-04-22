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
    "," : "COMMA",
    "-" : "HYPEN",
    "’" : "SINGLE_QUOTE",
}

def convert(name, input_string):

    print(f"        {name}: {name} {{")
    print('            compatible = "zmk,behavior-macro";')
    print("            #binding-cells = <0>;")
    print("            bindings")

    first_entry = "                ="
    same_line_entry = ","
    new_line_entry = "                ,"

    line_header = first_entry

    for char in input_string:
        if char.isupper():
            print(f"{line_header} <&macro_press &kp LSHFT>", end="")
            # everything after the first line uses the "same_line_entry"
            line_header = same_line_entry
            print(f"{line_header} <&macro_tap &kp {char}>", end="")
            print(f"{line_header} <&macro_release &kp LSHFT>", end="")

        elif char.isalpha():
            print(f"{line_header} <&macro_tap &kp {char.upper()}>", end="")

        elif char in char_to_kc.keys():
            print(f"{line_header} <&macro_tap &kp {char_to_kc[char]}>", end="")

        elif char.isnumeric():
            print(f"{line_header} <&macro_tap &kp N{char}>", end="")
        else:
            raise RuntimeError(f"TODO: not implemented. cannot handle:{char}")

        # everything after the first line uses the "same_line_entry"
        line_header = same_line_entry

    print("")
    print("                ;")
    print("        };")

convert("test_string", test_macro_string)
print("")