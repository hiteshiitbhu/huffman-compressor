#!/bin/bash
set -e

# Compile the application
echo "Building the Huffman Compressor..."
make clean
make

echo "--------------------------------------------------"
echo "Running Tests..."
echo "--------------------------------------------------"

# Setup test files
TEMP_DIR=$(mktemp -d -p .)
mkdir -p "$TEMP_DIR"

# Clean up temp directory on exit
trap 'rm -rf "$TEMP_DIR"' EXIT

# Test helper function
run_test() {
    local test_name="$1"
    local input_file="$2"
    local comp_file="$TEMP_DIR/compressed.bin"
    local decomp_file="$TEMP_DIR/decompressed.txt"
    
    echo "Running Test: $test_name"
    
    # 1. Compress
    ./huffman -c "$input_file" "$comp_file"
    
    # 2. Decompress
    ./huffman -d "$comp_file" "$decomp_file"
    
    # 3. Verify integrity
    if diff -q "$input_file" "$decomp_file" > /dev/null; then
        echo "✅ Verification PASSED: Files are identical."
    else
        echo "❌ Verification FAILED: Files differ."
        diff "$input_file" "$decomp_file"
        exit 1
    fi
    
    # 4. Show size metrics
    local orig_size=$(wc -c < "$input_file" | tr -d ' ')
    local comp_size=$(wc -c < "$comp_file" | tr -d ' ')
    
    if [ "$orig_size" -eq 0 ]; then
        echo "Original Size: $orig_size bytes | Compressed Size: $comp_size bytes"
    else
        local ratio=$(awk "BEGIN {print ($comp_size / $orig_size) * 100}")
        printf "Original Size: %d bytes | Compressed Size: %d bytes | Ratio: %.2f%%\n" "$orig_size" "$comp_size" "$ratio"
    fi
    echo "--------------------------------------------------"
}

# --- Test Case 1: Standard Text ---
cat << 'EOF' > "$TEMP_DIR/sample.txt"
Huffman coding is an entropy encoding algorithm used for lossless data compression. 
The algorithm was developed by David A. Huffman while he was a Ph.D. student at MIT.
It was published in the 1952 paper "A Method for the Construction of Minimum-Redundancy Codes".
This project demonstrates binary tree generation, priority queues, and byte-packing logic.
EOF
run_test "Standard Text File" "$TEMP_DIR/sample.txt"

# --- Test Case 2: Empty File ---
touch "$TEMP_DIR/empty.txt"
run_test "Empty File (Edge Case)" "$TEMP_DIR/empty.txt"

# --- Test Case 3: Single Repeating Character ---
python3 -c "print('A' * 1000, end='')" > "$TEMP_DIR/single_char.txt"
run_test "Single Character Repeating (Edge Case)" "$TEMP_DIR/single_char.txt"

# --- Test Case 4: Binary/Non-ASCII characters ---
# Write some arbitrary non-ASCII bytes
printf "\x00\x01\x02\xff\xfe\xfdHello\xe2\x82\xacWorld\x0a" > "$TEMP_DIR/binary_data.bin"
run_test "Binary/Non-ASCII Data" "$TEMP_DIR/binary_data.bin"

# --- Test Case 5: Large Repeated Text File ---
python3 -c "print('Huffman coding is an entropy encoding algorithm used for lossless data compression. The algorithm was developed by David A. Huffman while he was a Ph.D. student at MIT. It was published in the 1952 paper.\n' * 500)" > "$TEMP_DIR/large_text.txt"
run_test "Large Text File" "$TEMP_DIR/large_text.txt"

echo "🎉 All verification tests completed successfully!"
