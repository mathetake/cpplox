#ifndef cpplox_debug_h
#define cpplox_debug_h

#include "chunk.hpp"

void disassembleChunk(Chunk* chunk, const char* name);
int disassembleInstruction(Chunk* chunk, int offset);

#endif