#!/usr/bin/env python3

import pathlib


notes_header = pathlib.Path("../../../include/dt-bindings/zmk/midi.h")

midi_scale_map = {}
midi_scale_array = []


CAT_MAJOR = "major"
CAT_MINOR = "minor"
CAT_SEVENTH = "seventh"
CAT_MAJOR_SEVENTH = "major_seventh"
CAT_MINOR_SEVENTH = "minor_seventh"
CAT_AUGMENTED = "augmented"
CAT_DIMINISHED = "diminished"
CAT_MAJOR_MINOR_SEVENTH = "major_minor_seventh"

category_long_to_short = {
    CAT_MAJOR : "ma",
    CAT_MINOR : "mi",
    CAT_SEVENTH : "sev",
    # CAT_MAJOR_SEVENTH : "ma7",
    # CAT_MINOR_SEVENTH : "mi7",
    # CAT_AUGMENTED : "aug",
    # CAT_DIMINISHED : "dim",
    # CAT_MAJOR_MINOR_SEVENTH : "mm7",
}

category_long_to_right_layer = {
    CAT_MAJOR : "MA",
    CAT_MINOR : "MI",
    CAT_SEVENTH : "MA",
    # CAT_MAJOR_SEVENTH : "MA",
    # CAT_MINOR_SEVENTH : "MI",
    # CAT_AUGMENTED : "MA",
    # CAT_DIMINISHED : "MI",
    # CAT_MAJOR_MINOR_SEVENTH : "MA",
}



MAJOR_SCALE = "major"
MINOR_SCALE = "minor"

LEFT_LAYER_1 = "left_layer_1"
LEFT_LAYER_2 = "left_layer_2"

def read_header():

    global midi_scale_map
    global midi_scale_array

    with open(notes_header, "r") as file:
        for line in file:
            if line.startswith("#define NOTE_"):
                stripped = line.lstrip("#define ").rstrip()
                split = stripped.split(" ")
                if split[1].startswith("NOTE_"):
                    # map flats/sharps to the same value
                    value = midi_scale_map.get(split[1])
                    # if the value we are referencing isnt in the map yet, we will get None
                    assert value is not None
                    midi_scale_map[split[0]] = value
                    continue
                midi_scale_map[split[0]] = int(split[1], 16)

    # create a version of the map without duplicate values
    temp = []
    deduped = {}
    for key, val in midi_scale_map.items():
        if val not in temp:
            temp.append(val)
            deduped[key] = val

    # we can't guarantee that the
    # notes in the header are in numerical order, so sort to get our array
    tmp = sorted(deduped, key=deduped.get)
    midi_scale_array = tmp


def note_is_flat(note):
    return note.split("_")[1].endswith("b")
        
def note_is_sharp(note):
    return note.split("_")[1].endswith("s")


def translate_sharp_flat(note):

    translation_table = {
        "NOTE_Cs" : "NOTE_Db",
        "NOTE_Db" : "NOTE_Cs",
        "NOTE_Ds" : "NOTE_Eb",
        "NOTE_Eb" : "NOTE_Ds",
        "NOTE_Fs" : "NOTE_Gb",
        "NOTE_Gb" : "NOTE_Fs",
        "NOTE_Gs" : "NOTE_Ab",
        "NOTE_Ab" : "NOTE_Gs",
        "NOTE_As" : "NOTE_Bb",
        "NOTE_Bb" : "NOTE_As"
    }

    # chop off the octave tail
    split = note.split("_")
    tail = split[2]
    # ensure its tail is a number
    int(tail)

    base = f"{split[0]}_{split[1]}"
    assert base.startswith("NOTE_")

    trans_base = translation_table.get(base)
    assert trans_base is not None

    return f"{trans_base}_{tail}"

def lower_octave(note):
    # chop off the octave tail
    split = note.split("_")
    tail = split[2]
    # ensure its tail is a number
    tail = int(tail)

    tail = tail - 1

    return f"{split[0]}_{split[1]}_{tail}"

def raise_octave(note):
    # chop off the octave tail
    split = note.split("_")
    tail = split[2]
    # ensure its tail is a number
    tail = int(tail)

    tail = tail + 1

    return f"{split[0]}_{split[1]}_{tail}"

def find_note(root_note, offset):

    root_note_val = midi_scale_map.get(root_note)
    assert root_note_val is not None

    offset_note_val = root_note_val + offset

    offset_note = midi_scale_array[offset_note_val]
    
    return offset_note


# each major scale has a root note and then goes
# 2 2 1 2 2 2 1
def major_scale_notes(root_note):

    offset_set = [2, 2, 1, 2, 2, 2, 1]

    enforce_sharp = False
    enforce_flat = False

    enforce_flat = note_is_flat(root_note)
    enforce_sharp = note_is_sharp(root_note)

    scale = []
    scale.append(root_note)
    previous_note = root_note

    for offset in offset_set:
        cur_note = find_note(previous_note, offset)

        if enforce_sharp and note_is_flat(cur_note):
            cur_note = translate_sharp_flat(cur_note)
        elif enforce_flat and note_is_sharp(cur_note):
            cur_note = translate_sharp_flat(cur_note)

        scale.append(cur_note)
        previous_note = cur_note

    return scale


# each major scale has a root note and then goes
# 2 1 2 2 1 2 2
def minor_scale_notes(root_note):

    offset_set = [2, 1, 2, 2, 1, 2, 2]
    enforce_sharp = False
    enforce_flat = False

    enforce_flat = note_is_flat(root_note)
    enforce_sharp = note_is_sharp(root_note)

    scale = []
    scale.append(root_note)
    previous_note = root_note

    for offset in offset_set:
        cur_note = find_note(previous_note, offset)

        if enforce_sharp and note_is_flat(cur_note):
            cur_note = translate_sharp_flat(cur_note)
        elif enforce_flat and note_is_sharp(cur_note):
            cur_note = translate_sharp_flat(cur_note)

        scale.append(cur_note)
        previous_note = cur_note

    return scale


def fill_keymap_template(scale, scale_type, left_layer):

    sixth_oct_down = lower_octave(scale[5])
    seventh_oct_down = lower_octave(scale[6])

    sixth_oct_dn_dn = lower_octave(sixth_oct_down)

    root_clean = scale[0].split("_")[1]


    left_layer_1_row_1 = "&mo MA_Ab  &mo MA_Eb  &mo MA_Bb &mo MA_F &mo MA_C"
    left_layer_1_row_2 = "&mo MI_Ab  &mo MI_Eb  &mo MI_Bb &mo MI_F &mo MI_C"
    left_layer_1_row_3 = "&trans  &trans  &trans  &trans  &trans"

    left_layer_2_row_1 = "&mo MA_G  &mo MA_D  &mo MA_A &mo MA_E &mo MA_B"
    left_layer_2_row_2 = "&mo MI_G  &mo MI_D  &mo MI_A &mo MI_E &mo MI_B"
    left_layer_2_row_3 = "&trans  &trans  &trans  &trans  &trans"

    if left_layer == LEFT_LAYER_1:
        left_layer_row_1 = left_layer_1_row_1
        left_layer_row_2 = left_layer_1_row_2
        left_layer_row_3 = left_layer_1_row_3


    if left_layer == LEFT_LAYER_2:
        left_layer_row_1 = left_layer_2_row_1
        left_layer_row_2 = left_layer_2_row_2
        left_layer_row_3 = left_layer_2_row_3


# the two extra notes on the left of the main chord breakdown are:
# "Y" aka the 7th of the octave below
# "H" aka the 6th of the octave below

    KEYMAP_TEMPLATE = f"""
        {root_clean}_{scale_type} {{
bindings = <
   {left_layer_row_1}                           &midi {seventh_oct_down}   &midi {scale[1]}   &midi {scale[3]}  &midi {scale[5]}     &midi {scale[7]}
   {left_layer_row_2}                           &midi {sixth_oct_down}   &midi {scale[0]}   &midi {scale[2]}  &midi {scale[4]}     &midi {scale[6]}
   {left_layer_row_3}                                      &midi {sixth_oct_dn_dn}   &midi {lower_octave(scale[0])}   &midi {lower_octave(scale[2])}  &midi {lower_octave(scale[4])}     &midi {lower_octave(scale[6])}
   &to DEFAULT_L   &none  &none                                                                          &none  &none     &none
                                             &trans  &trans &trans     &trans  &trans &trans
                                                    &trans &trans     &trans  &trans
            >;
        }};
    """

    return KEYMAP_TEMPLATE



def fill_chord_macro_template(notes, category):

    root_clean = notes[0].split("_")[1]
    short = category_long_to_short.get(category)
    assert short is not None
    short_name = f"ch_{root_clean}_{short}"

    right_layer = category_long_to_right_layer.get(category)
    assert right_layer is not None
    layer_name = f"{right_layer}_{root_clean}"

    long_name = f"{root_clean}_{category}"

    if category in [CAT_MAJOR, CAT_MINOR, CAT_AUGMENTED, CAT_DIMINISHED]:
        # 3 notes in each chord
        MACRO_TEMPLATE = f"""
// macros for each chord key, so we can switch to the chords layer and press the chords keys at the same time
        {short_name}: {long_name} {{
            compatible = "zmk,behavior-macro";
            wait-ms = <2>;
            tap-ms = <0>;
            #binding-cells = <0>;
            bindings
            = <&macro_press &mo {layer_name}>
            , <&macro_press &midi {notes[0]}>
            , <&macro_press &midi {notes[1]}>
            , <&macro_press &midi {notes[2]}>
            , <&macro_pause_for_release>
            , <&macro_release &mo {layer_name}>
            , <&macro_release &midi {notes[0]}>
            , <&macro_release &midi {notes[1]}>
            , <&macro_release &midi {notes[2]}>
            ;
        }};
        """
    elif category in [CAT_SEVENTH, CAT_MAJOR_SEVENTH, CAT_MINOR_SEVENTH, CAT_MAJOR_MINOR_SEVENTH]:
        # 4 notes in each chord
        MACRO_TEMPLATE = f"""
// macros for each chord key, so we can switch to the chords layer and press the chords keys at the same time
        {short_name}: {long_name} {{
            compatible = "zmk,behavior-macro";
            wait-ms = <2>;
            tap-ms = <0>;
            #binding-cells = <0>;
            bindings
            = <&macro_press &mo {layer_name}>
            , <&macro_press &midi {notes[0]}>
            , <&macro_press &midi {notes[1]}>
            , <&macro_press &midi {notes[2]}>
            , <&macro_press &midi {notes[3]}>
            , <&macro_pause_for_release>
            , <&macro_release &mo {layer_name}>
            , <&macro_release &midi {notes[0]}>
            , <&macro_release &midi {notes[1]}>
            , <&macro_release &midi {notes[2]}>
            , <&macro_release &midi {notes[3]}>
            ;
        }};
        """
    else:
        raise RuntimeError("category {category} in unknwon")


    return MACRO_TEMPLATE


def get_scale(root_note, scale_type):
    if scale_type == MAJOR_SCALE:
        scale = major_scale_notes(root_note)
    elif scale_type == MINOR_SCALE:
        scale = minor_scale_notes(root_note)

    return scale

def get_chord(root_note, category):

    if category == CAT_MAJOR:
        offset_set = [4, 7]
    elif category == CAT_MINOR:
        offset_set = [3, 7]
    elif category == CAT_SEVENTH:
        offset_set = [4, 7, 10]
    elif category == CAT_MAJOR_SEVENTH:
        offset_set = [4, 7, 11]
    elif category == CAT_MINOR_SEVENTH:
        offset_set = [3, 7, 10]
    elif category == CAT_AUGMENTED:
        offset_set = [4, 8]
    elif category == CAT_DIMINISHED:
        offset_set = [3, 6]
    elif category == CAT_MAJOR_MINOR_SEVENTH:
        offset_set = [3, 7, 11]

    chord = []
    chord.append(root_note)

    for offset in offset_set:
        cur_note = find_note(root_note, offset)
        chord.append(cur_note)

    return chord


def generate(root_note_list):

    macros = []
    layers = []


    for category in category_long_to_short.keys():
        for root_note in root_note_list:
            chord_notes = get_chord(root_note, category)
            chord_macro = fill_chord_macro_template(chord_notes, category)
            macros.append(chord_macro)


    count = 0
    for scale_type in [MAJOR_SCALE, MINOR_SCALE]:
        left_layer = LEFT_LAYER_1
        for root_note in root_note_list:
            if count >= 5:
                left_layer = LEFT_LAYER_2
            scale = get_scale(root_note, scale_type)
            layer = fill_keymap_template(scale, scale_type, left_layer)
            layers.append(layer)
            count += 1

    for macro in macros:
        print(macro)

    for layer in layers:
        print(layer)






read_header()


root_note_list = ["NOTE_Ab_5",
              "NOTE_Eb_5",
              "NOTE_Bb_5",
              "NOTE_F_5",
              "NOTE_C_5",
              "NOTE_G_5",
              "NOTE_D_5",
              "NOTE_A_5",
              "NOTE_E_5",
              "NOTE_B_5"]

generate(root_note_list)
