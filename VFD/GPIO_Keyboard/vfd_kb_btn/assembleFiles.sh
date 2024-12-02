#!/bin/bash

# Define the output master file
output_file="cs50x_master_script.log"

# Clear the output file if it exists
> "$output_file"

# Create a temporary file with the list of .c and .h files, excluding backup files
find . -maxdepth 1 -type f \( -name "*.c" -o -name "*.h" \) ! -name "*~" | sort > temp_files.txt

# Flag to check if the first file is being processed
first_file=true

# Read each file and append contents to master script
while IFS= read -r file; do
    if [[ -f "$file" ]]; then
        # Add a demarcation line only if it's not the first file
        if [[ "$first_file" = false ]]; then
            echo -e "\r\n\r\n\r\n" >> "$output_file"
        else
            first_file=false
        fi
        
        # Append the demarcation line, file name, and second demarcation line
        echo -e "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -" >> "$output_file"
        echo -e "# File: $file" >> "$output_file"
        echo -e "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -" >> "$output_file"
        
        # Append the content of the current file with Windows-style line endings
        cat "$file" | sed 's/$/\r/' >> "$output_file"
    else
        echo "Warning: File $file does not exist."
    fi
done < temp_files.txt

# Clean up temporary file
rm temp_files.txt

echo "Master script generated as $output_file"
