import sys
import struct

if len(sys.argv) == 1:
    sys.exit("Must include a file to process")

with open(sys.argv[1]) as input_file:
    header_file = ""
    data_file = ""
    identifier = ""

    text_dict = {}
    count = 0

    for line in input_file.readlines():
        if count == 0:
            header_file = line.strip()
        elif count == 1:
            data_file = line.strip()
        elif count == 2:
            identifier = line.strip()
        else:
            mapping = line.split(',')
            text_dict[mapping[0]] = mapping[1].encode()

        count += 1

    binary_size = 0

    with open(header_file, 'w') as header:
        with open(data_file, 'wb') as data:
            header.write("#pragma once\n\n")
            header.write("enum\n")
            header.write("{\n")

            data.write(struct.pack('h', len(text_dict)))
            binary_size += 2 + 4 * (len(text_dict))

            text_items = text_dict.items()
            for k, v in text_items:
                data.write(struct.pack('h', binary_size))
                data.write(struct.pack('h', len(v)))
                binary_size += len(v)

            for k, v in text_items:
                header.write("\t" + k + ",\n")
                data.write(v)

            header.write("};\n")

