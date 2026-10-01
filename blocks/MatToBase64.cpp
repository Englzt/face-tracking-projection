#include "MatToBase64.hpp"

static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";


static inline bool is_base64(BYTE c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}

/**
Encodes a BYTE buffer to a Base64 String.
@param buf Input buffer.
@param bufLen Buffer size.
*/
std::string base64_encode(std::vector<BYTE> buf) {
    std::string ret;
    int i = 0;
    int j = 0;
    BYTE char_array_3[3];
    BYTE char_array_4[4];

    for (BYTE& b : buf) {
        char_array_3[i++] = b;
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; (i < 4); i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i)
    {
        for (j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        while ((i++ < 3))
            ret += '=';
    }

    return ret;
}

/**
Decodes a Base64 String to dynamice BYTE array.
@param encoded_string String to be decoded.
*/
std::vector<BYTE> base64_decode(std::string const& encoded_string) {
    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    BYTE char_array_4[4], char_array_3[3];
    std::vector<BYTE> ret;

    while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_]; in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret.push_back(char_array_3[i]);
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++)
            char_array_4[j] = 0;

        for (j = 0; j < 4; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++) ret.push_back(char_array_3[j]);
    }

    return ret;
}

/**
Converts a Surface8u to a Base64 encoded string.
@param img Image surface to be converted.
@param ext File extension that defines the output format.
*/
std::string surface8uToBase64(ci::Surface8u img, cv::String ext) {
    cv::Mat input(toOcv(img));
    return matToBase64(input, ext);
}

/**
Converts a Mat to a Base64 encoded string.
@param imgMat Image to be converted.
@param ext File extension that defines the output format.
*/
std::string matToBase64(cv::Mat imgMat, cv::String ext, int quality, bool scale, int newWidth) {
    std::vector<BYTE> buf;

    // Kopie anlegen, um das Original nicht zu verändern
    cv::Mat safeMat = imgMat;

    if (scale && safeMat.cols > newWidth) {
        float hScale = (float)safeMat.cols / (float)safeMat.rows;
        cv::resize(safeMat, safeMat, cv::Size(newWidth, newWidth * hScale));
    }

    if (ext == ".jpg") {
        std::vector<int> param(2);
        param[0] = cv::IMWRITE_JPEG_QUALITY;
        param[1] = quality;
        cv::imencode(ext, safeMat, buf, param);
    } else {
        cv::imencode(ext, safeMat, buf);
    }
    //const BYTE* enc_msg = reinterpret_cast<BYTE*>(buf.data());
    return base64_encode(buf);
}