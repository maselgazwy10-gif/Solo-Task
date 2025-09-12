#include <iostream>
#include <string>
#include <random>
#include <bitset>
#include <vector>
#include <cstddef> // For size_t

using namespace std;

// CRC-8 polynomial: x^8 + x^2 + x + 1 (0x107)
const string CRC_POLYNOMIAL = "100000111";

// Function to perform XOR operation on two bits
char xor_op(char a, char b) {
    return (a == b) ? '0' : '1';
}

// Function to perform modulo-2 division
string mod2div(string dividend, string divisor) {
    int pick = divisor.length();
    string tmp = dividend.substr(0, pick);
    
    int n = dividend.length();
    
    while (pick < n) {
        if (tmp[0] == '1') {
            // XOR with divisor
            for (int i = 0; i < divisor.length(); i++) {
                tmp[i] = xor_op(tmp[i], divisor[i]);
            }
        }
        
        // Remove the first bit and bring down the next bit
        tmp = tmp.substr(1) + dividend[pick];
        pick++;
    }
    
    // Last step
    if (tmp[0] == '1') {
        for (int i = 0; i < divisor.length(); i++) {
            tmp[i] = xor_op(tmp[i], divisor[i]);
        }
    }
    
    return tmp.substr(1);
}

// Function to calculate CRC
string calculate_crc(string data, string polynomial) {
    int l_poly = polynomial.length();
    string appended_data = data + string(l_poly - 1, '0');
    return mod2div(appended_data, polynomial);
}

// Function to introduce random errors
string introduce_error(string data, double error_probability) {
    static mt19937 gen(time(0));
    uniform_real_distribution<double> dis(0.0, 1.0);
    
    string result = data;
    for (int i = 0; i < result.length(); i++) {
        if (dis(gen) < error_probability) {
            result[i] = (result[i] == '0') ? '1' : '0';
        }
    }
    return result;
}

// Function to convert ASCII string to binary
string ascii_to_binary(const string& text) {
    string binary;
    for (char c : text) {
        binary += bitset<8>(c).to_string();
    }
    return binary;
}

// Function to convert binary to ASCII string
string binary_to_ascii(const string& binary) {
    string text;
    for (size_t i = 0; i < binary.length(); i += 8) {
        string byte = binary.substr(i, 8);
        if (byte.length() == 8) {
            text += static_cast<char>(bitset<8>(byte).to_ulong());
        }
    }
    return text;
}

// Function to split data into 8-byte (64-bit) chunks
vector<string> split_into_chunks(const string& binary_data, int chunk_size_bits) {
    vector<string> chunks;
    for (size_t i = 0; i < binary_data.length(); i += chunk_size_bits) {
        string chunk = binary_data.substr(i, chunk_size_bits);
        // Pad the last chunk with zeros if needed
        if (chunk.length() < chunk_size_bits) {
            chunk.append(chunk_size_bits - chunk.length(), '0');
        }
        chunks.push_back(chunk);
    }
    return chunks;
}

int main() {
    cout << "=== Bob's Binary Confession Error Detection System ===\n";
    cout << "Using CRC Polynomial: " << CRC_POLYNOMIAL << " (CRC-8)\n\n";
    
    // Bob's heartfelt message
    string message = "I love you";
    cout << "Bob's Original Message: \"" << message << "\"\n";
    
    // Convert to binary
    string full_binary_message = ascii_to_binary(message);
    cout << "Full Binary Message (" << full_binary_message.length() << " bits):\n";
    for (int i = 0; i < full_binary_message.length(); i++) {
        if (i % 8 == 0 && i != 0) cout << " ";
        cout << full_binary_message[i];
    }
    cout << "\n\n";
    
    // Split into 8-byte (64-bit) chunks
    const int BURST_SIZE_BITS = 64; // 8 bytes
    vector<string> chunks = split_into_chunks(full_binary_message, BURST_SIZE_BITS);
    cout << "Message split into " << chunks.size() << " burst(s) of " << BURST_SIZE_BITS << " bits each.\n\n";
    
    // Process each burst
    for (size_t burst_num = 0; burst_num < chunks.size(); burst_num++) {
        cout << "--- Processing Burst " << burst_num + 1 << " ---\n";
        
        string binary_burst = chunks[burst_num];
        cout << "Burst Data (64 bits):\n";
        for (int j = 0; j < binary_burst.length(); j++) {
            if (j % 8 == 0 && j != 0) cout << " ";
            cout << binary_burst[j];
        }
        cout << "\n";
        
        // Calculate CRC for this burst
        string crc_code = calculate_crc(binary_burst, CRC_POLYNOMIAL);
        cout << "Calculated CRC Code for burst (8 bits): " << crc_code << "\n";
        
        // Create frame to send for this burst
        string frame_to_send = binary_burst + crc_code;
        cout << "Frame to Send (72 bits):\n";
        for (int j = 0; j < frame_to_send.length(); j++) {
            if (j % 8 == 0 && j != 0) cout << " ";
            cout << frame_to_send[j];
        }
        cout << "\n";
        
        // Simulate transmission with potential errors
        cout << "\n--- Transmission through Egyptian Network ---\n";
        string received_frame = introduce_error(frame_to_send, 0.1); // 10% error probability
        cout << "Received Frame (72 bits, possibly corrupted):\n";
        for (int j = 0; j < received_frame.length(); j++) {
            if (j % 8 == 0 && j != 0) cout << " ";
            cout << received_frame[j];
        }
        cout << "\n";
        
        // Alice's side: extract data and CRC for this burst
        string received_data = received_frame.substr(0, BURST_SIZE_BITS);
        string received_crc = received_frame.substr(BURST_SIZE_BITS);
        
        cout << "Received Data (64 bits):\n";
        for (int j = 0; j < received_data.length(); j++) {
            if (j % 8 == 0 && j != 0) cout << " ";
            cout << received_data[j];
        }
        cout << "\n";
        cout << "Received CRC (8 bits): " << received_crc << "\n";
        
        // Alice calculates CRC on received data
        string calculated_crc = calculate_crc(received_data, CRC_POLYNOMIAL);
        cout << "CRC Calculated by Alice (8 bits): " << calculated_crc << "\n\n";
        
        // Verification for this burst
        cout << "--- Verification Results for Burst " << burst_num + 1 << " ---\n";
        if (calculated_crc == received_crc) {
            cout << "✅ SUCCESS: No error detected in burst.\n";
        } else {
            cout << "❌ ERROR: Checksum mismatch in burst! This burst needs to be resent.\n";
        }
        cout << "----------------------------------------\n\n";
    }
    
    return 0;
}