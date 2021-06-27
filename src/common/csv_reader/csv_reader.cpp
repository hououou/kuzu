#include "src/common/include/csv_reader/csv_reader.h"

#include <fstream>

#include "src/common/include/configs.h"
#include "src/common/include/date.h"
#include "src/common/include/utils.h"

namespace graphflow {
namespace common {

CSVReader::CSVReader(const string& fname, const char tokenSeparator, uint64_t blockId)
    : tokenSeparator(tokenSeparator), line{new char[1 << 10]},
      readingBlockIdx(CSV_READING_BLOCK_SIZE * blockId),
      readingBlockEndIdx(CSV_READING_BLOCK_SIZE * (blockId + 1)) {
    openFile(fname);
    auto isBeginningOfLine = false;
    if (0 == readingBlockIdx) {
        isBeginningOfLine = true;
    } else {
        fseek(f, readingBlockIdx - 1, SEEK_SET);
        if ('\n' == fgetc(f)) {
            isBeginningOfLine = true;
        }
    }
    if (!isBeginningOfLine) {
        while ('\n' != fgetc(f)) {}
    }
}

CSVReader::CSVReader(const string& fname, const char tokenSeparator)
    : tokenSeparator(tokenSeparator), line{new char[1 << 10]} {
    openFile(fname);
    readingBlockIdx = 0;
    readingBlockEndIdx = UINT64_MAX;
}

CSVReader::~CSVReader() {
    fclose(f);
    delete[](line);
}

bool CSVReader::hasNextLine() {
    // the block has already been ended, return false.
    if (isEndOfBlock) {
        return false;
    }
    // the previous line has not been processed yet, return true.
    if (nextLineIsNotProcessed) {
        return true;
    }
    // file cursor is past the block limit, end the block, return false.
    if (ftell(f) >= readingBlockEndIdx) {
        isEndOfBlock = true;
        return false;
    }
    // else, read the next line.
    lineLen = getline(&line, &lineCapacity, f);
    while (2 > lineLen || '#' == line[0]) {
        lineLen = getline(&line, &lineCapacity, f);
    };
    linePtrStart = linePtrEnd = -1;
    // file ends, end the file.
    if (feof(f)) {
        isEndOfBlock = true;
        return false;
    }
    return true;
}

void CSVReader::skipLine() {
    nextLineIsNotProcessed = false;
}

bool CSVReader::skipTokenIfNull() {
    if (linePtrEnd - linePtrStart == 0) {
        nextLineIsNotProcessed = false;
        return true;
    }
    return false;
}

void CSVReader::skipToken() {
    setNextTokenIsNotProcessed();
}

bool CSVReader::hasNextToken() {
    if (nextTokenIsNotProcessed) {
        return true;
    }
    linePtrEnd++;
    linePtrStart = linePtrEnd;
    if (linePtrEnd == lineLen) {
        nextLineIsNotProcessed = false;
        return false;
    }
    nextTokenLen = 0;
    while (tokenSeparator != line[linePtrEnd] && '\n' != line[linePtrEnd]) {
        nextTokenLen++;
        linePtrEnd++;
    }
    line[linePtrEnd] = 0;
    return true;
}
uint64_t CSVReader::getNextTokenLen() {
    return nextTokenLen;
}

int32_t CSVReader::getInt32() {
    setNextTokenIsNotProcessed();
    return convertToInt32(line + linePtrStart);
}

double_t CSVReader::getDouble() {
    setNextTokenIsNotProcessed();
    return convertToDouble(line + linePtrStart);
}

uint8_t CSVReader::getBoolean() {
    setNextTokenIsNotProcessed();
    return convertToBoolean(line + linePtrStart);
}

char* CSVReader::getString() {
    setNextTokenIsNotProcessed();
    return line + linePtrStart;
}

date_t CSVReader::getDate() {
    date_t retVal = Date::FromCString(line + linePtrStart, nextTokenLen);
    setNextTokenIsNotProcessed();
    return retVal;
}

void CSVReader::setNextTokenIsNotProcessed() {
    nextTokenIsNotProcessed = false;
    nextTokenLen = -1;
}

void CSVReader::openFile(string fname) {
    f = fopen(fname.c_str(), "r");
    if (nullptr == f) {
        throw invalid_argument("Cannot open file: " + fname);
    }
}

} // namespace common
} // namespace graphflow
