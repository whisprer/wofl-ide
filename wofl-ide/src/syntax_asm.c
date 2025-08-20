// ==================== syntax_asm.c ====================
// Assembly language syntax highlighting (x86/x64)

#include "editor.h"
#include <wctype.h>

// x86/x64 instructions (common subset)
static const wchar_t* asm_instructions[] = {
    // Data movement
    L"mov", L"movb", L"movw", L"movl", L"movq", L"movsx", L"movzx", L"lea",
    L"push", L"pop", L"xchg", L"bswap", L"xadd", L"cmpxchg",
    // Arithmetic
    L"add", L"sub", L"mul", L"imul", L"div", L"idiv", L"inc", L"dec", L"neg",
    L"adc", L"sbb", L"cmp",
    // Logical
    L"and", L"or", L"xor", L"not", L"test", L"shl", L"shr", L"sal", L"sar",
    L"rol", L"ror", L"rcl", L"rcr", L"shld", L"shrd",
    // Control flow
    L"jmp", L"je", L"jne", L"jz", L"jnz", L"ja", L"jae", L"jb", L"jbe",
    L"jg", L"jge", L"jl", L"jle", L"js", L"jns", L"jo", L"jno", L"jc", L"jnc",
    L"call", L"ret", L"loop", L"loope", L"loopne", L"int", L"iret",
    // String operations
    L"movs", L"cmps", L"scas", L"lods", L"stos", L"rep", L"repe", L"repne",
    // Stack operations
    L"enter", L"leave", L"pusha", L"popa", L"pushf", L"popf",
    // System
    L"nop", L"hlt", L"cli", L"sti", L"syscall", L"sysenter", L"sysexit",
    // SSE/AVX
    L"movaps", L"movups", L"movapd", L"movupd", L"addps", L"subps", L"mulps",
    L"divps", L"xorps", L"orps", L"andps"
};

// x86/x64 registers
static const wchar_t* asm_registers[] = {
    // 64-bit
    L"rax", L"rbx", L"rcx", L"rdx", L"rsi", L"rdi", L"rbp", L"rsp",
    L"r8", L"r9", L"r10", L"r11", L"r12", L"r13", L"r14", L"r15",
    // 32-bit
    L"eax", L"ebx", L"ecx", L"edx", L"esi", L"edi", L"ebp", L"esp",
    L"r8d", L"r9d", L"r10d", L"r11d", L"r12d", L"r13d", L"r14d", L"r15d",
    // 16-bit
    L"ax", L"bx", L"cx", L"dx", L"si", L"di", L"bp", L"sp",
    L"r8w", L"r9w", L"r10w", L"r11w", L"r12w", L"r13w", L"r14w", L"r15w",
    // 8-bit
    L"al", L"bl", L"cl", L"dl", L"ah", L"bh", L"ch", L"dh",
    L"sil", L"dil", L"bpl", L"spl",
    L"r8b", L"r9b", L"r10b", L"r11b", L"r12b", L"r13b", L"r14b", L"r15b",
    // Segment registers
    L"cs", L"ds", L"es", L"fs", L"gs", L"ss",
    // Special registers
    L"rip", L"eip", L"ip", L"rflags", L"eflags", L"flags",
    // SSE/AVX registers
    L"xmm0", L"xmm1", L"xmm2", L"xmm3", L"xmm4", L"xmm5", L"xmm6", L"xmm7",
    L"xmm8", L"xmm9", L"xmm10", L"xmm11", L"xmm12", L"xmm13", L"xmm14", L"xmm15",
    L"ymm0", L"ymm1", L"ymm2", L"ymm3", L"ymm4", L"ymm5", L"ymm6", L"ymm7"
};

// Assembler directives
static const wchar_t* asm_directives[] = {
    L"section", L"global", L"extern", L"db", L"dw", L"dd", L"dq", L"dt",
    L"resb", L"resw", L"resd", L"resq", L"rest", L"equ", L"times",
    L"align", L"org", L"bits", L"use16", L"use32", L"use64",
    L"segment", L"ends", L"proc", L"endp", L"macro", L"endm",
    L"include", L"incbin", L"define", L"undef", L"ifdef", L"ifndef", L"endif"
};

static bool is_asm_instruction(const wchar_t *word, int len) {
    for (size_t i = 0; i < sizeof(asm_instructions) / sizeof(asm_instructions[0]); i++) {
        if ((int)wcslen(asm_instructions[i]) == len && 
            _wcsnicmp(asm_instructions[i], word, len) == 0) {
            return true;
        }
    }
    return false;
}

static bool is_asm_register(const wchar_t *word, int len) {
    for (size_t i = 0; i < sizeof(asm_registers) / sizeof(asm_registers[0]); i++) {
        if ((int)wcslen(asm_registers[i]) == len && 
            _wcsnicmp(asm_registers[i], word, len) == 0) {
            return true;
        }
    }
    return false;
}

static bool is_asm_directive(const wchar_t *word, int len) {
    for (size_t i = 0; i < sizeof(asm_directives) / sizeof(asm_directives[0]); i++) {
        if ((int)wcslen(asm_directives[i]) == len && 
            _wcsnicmp(asm_directives[i], word, len) == 0) {
            return true;
        }
    }
    return false;
}

void syntax_scan_asm(const wchar_t *line, int len, TokenSpan *out, int *out_n) {
    int i = 0, m = 0;
    
    while (i < len && m < WOFL_MAX_TOKENS - 1) {
        wchar_t ch = line[i];
        
        // Whitespace
        if (iswspace(ch)) {
            int start = i++;
            while (i < len && iswspace(line[i])) i++;
            out[m++] = (TokenSpan){start, i - start, TK_TEXT};
        }
        // Comments (;)
        else if (ch == L';') {
            out[m++] = (TokenSpan){i, len - i, TK_COMMENT};
            break;
        }
        // String literals
        else if (ch == L'"' || ch == L'\'') {
            wchar_t quote = ch;
            int start = i++;
            while (i < len) {
                if (line[i] == L'\\') {
                    i += 2;
                } else if (line[i] == quote) {
                    i++;
                    break;
                } else {
                    i++;
                }
            }
            out[m++] = (TokenSpan){start, i - start, TK_STR};
        }
        // Hexadecimal numbers (0x...)
        else if (ch == L'0' && i + 1 < len && (line[i + 1] == L'x' || line[i + 1] == L'X')) {
            int start = i;
            i += 2;
            while (i < len && (iswxdigit(line[i]) || line[i] == L'_')) {
                i++;
            }
            out[m++] = (TokenSpan){start, i - start, TK_NUM};
        }
        // Binary numbers (0b...)
        else if (ch == L'0' && i + 1 < len && (line[i + 1] == L'b' || line[i + 1] == L'B')) {
            int start = i;
            i += 2;
            while (i < len && (line[i] == L'0' || line[i] == L'1' || line[i] == L'_')) {
                i++;
            }
            out[m++] = (TokenSpan){start, i - start, TK_NUM};
        }
        // Decimal numbers
        else if (iswdigit(ch)) {
            int start = i++;
            while (i < len && (iswdigit(line[i]) || line[i] == L'_')) {
                i++;
            }
            // Check for size suffix (b, w, d, q)
            if (i < len && (line[i] == L'b' || line[i] == L'w' || 
                           line[i] == L'd' || line[i] == L'q' || line[i] == L'h')) {
                i++;
            }
            out[m++] = (TokenSpan){start, i - start, TK_NUM};
        }
        // Labels (identifier followed by :)
        else if (iswalpha(ch) || ch == L'.' || ch == L'_') {
            int start = i++;
            while (i < len && (iswalnum(line[i]) || line[i] == L'.' || 
                              line[i] == L'_')) {
                i++;
            }
            
            // Check if it's a label
            bool is_label = (i < len && line[i] == L':');
            if (is_label) {
                i++; // Include the colon
                out[m++] = (TokenSpan){start, i - start, TK_IDENT};
            } else {
                // Check token type
                TokenSpan token = {start, i - start, TK_IDENT};
                
                if (is_asm_instruction(line + start, token.len)) {
                    token.cls = TK_KW;
                } else if (is_asm_register(line + start, token.len)) {
                    token.cls = TK_KW; // Use keyword color for registers
                } else if (is_asm_directive(line + start, token.len) || 
                          (start > 0 && line[start] == L'.')) {
                    token.cls = TK_COMMENT; // Use comment color for directives
                }
                
                out[m++] = token;
            }
        }
        // Memory addressing brackets
        else if (ch == L'[' || ch == L']') {
            out[m++] = (TokenSpan){i++, 1, TK_PUNCT};
        }
        // Other punctuation
        else {
            out[m++] = (TokenSpan){i++, 1, TK_PUNCT};
        }
    }
    
    *out_n = m;
}