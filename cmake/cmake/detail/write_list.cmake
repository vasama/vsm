# LIST_FILE: Output list file path.
# LIST_DATA: Semicolon separated list.

list(JOIN LIST_DATA "\n" file_data)
file(WRITE "${LIST_FILE}" "${file_data}\n")
