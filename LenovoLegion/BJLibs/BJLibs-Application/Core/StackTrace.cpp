// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "StackTrace.h"

#ifdef WINNT
#include <windows.h>
#include <DbgHelp.h>
#endif

#ifdef linux
#include <execinfo.h>
#endif

#include <sstream>


namespace bj { namespace framework {

#ifdef WINNT
StackTrace::Stack_T StackTrace::getStackTrace(size_t maxStackTraceSize) {

    Stack_T stack;



    HANDLE process = GetCurrentProcess();
    HANDLE thread = GetCurrentThread();

    CONTEXT context;
    memset(&context, 0, sizeof(CONTEXT));
    context.ContextFlags = CONTEXT_FULL;
    RtlCaptureContext(&context);

    SymInitialize(process, NULL, TRUE);

    DWORD image;
    STACKFRAME64 stackframe;
    ZeroMemory(&stackframe, sizeof(STACKFRAME64));

  #ifdef _M_IX86
    image = IMAGE_FILE_MACHINE_I386;
    stackframe.AddrPC.Offset = context.Eip;
    stackframe.AddrPC.Mode = AddrModeFlat;
    stackframe.AddrFrame.Offset = context.Ebp;
    stackframe.AddrFrame.Mode = AddrModeFlat;
    stackframe.AddrStack.Offset = context.Esp;
    stackframe.AddrStack.Mode = AddrModeFlat;
  #elif _M_X64
    image = IMAGE_FILE_MACHINE_AMD64;
    stackframe.AddrPC.Offset = context.Rip;
    stackframe.AddrPC.Mode = AddrModeFlat;
    stackframe.AddrFrame.Offset = context.Rsp;
    stackframe.AddrFrame.Mode = AddrModeFlat;
    stackframe.AddrStack.Offset = context.Rsp;
    stackframe.AddrStack.Mode = AddrModeFlat;
  #elif _M_IA64
    image = IMAGE_FILE_MACHINE_IA64;
    stackframe.AddrPC.Offset = context.StIIP;
    stackframe.AddrPC.Mode = AddrModeFlat;
    stackframe.AddrFrame.Offset = context.IntSp;
    stackframe.AddrFrame.Mode = AddrModeFlat;
    stackframe.AddrBStore.Offset = context.RsBSP;
    stackframe.AddrBStore.Mode = AddrModeFlat;
    stackframe.AddrStack.Offset = context.IntSp;
    stackframe.AddrStack.Mode = AddrModeFlat;
  #endif

    for (size_t i = 0; i < maxStackTraceSize; i++) {

      BOOL result = StackWalk64(
        image, process, thread,
        &stackframe, &context, NULL,
        SymFunctionTableAccess64, SymGetModuleBase64, NULL);

      if (!result) { break; }

      char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
      PSYMBOL_INFO symbol = (PSYMBOL_INFO)buffer;
      symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
      symbol->MaxNameLen = MAX_SYM_NAME;

      DWORD64 displacement = 0;
      if (SymFromAddr(process, stackframe.AddrPC.Offset, &displacement, symbol)) {
          stack.push_back(StackTrace::Frame_T {i,symbol->Name,symbol->Address});
      } else {
          stack.push_back(StackTrace::Frame_T{i,"???",symbol->Address});
      }

    }

    SymCleanup(process);

    return stack;
}
#endif

#ifdef linux
StackTrace::Stack_T StackTrace::getStackTrace(size_t maxStackTraceSize) {
    Stack_T stack;
    void ** array = new void*[maxStackTraceSize];

    int size = backtrace(array,static_cast<int>(maxStackTraceSize));

    char **strings = backtrace_symbols(array, size);

    for (int i = 0; i < size; i++) {
        stack.push_back(StackTrace::Frame_T{static_cast<uint64_t>(i),strings[i],reinterpret_cast<uint64_t>(array[i])});
    }

    free(strings);
    delete []array;

    return stack;
}
#endif


QString StackTrace::getFormatetStackTrace()
{
    std::stringstream buffer;

    buffer << getStackTrace();

    return QString(buffer.str().c_str());
}


std::ostream& operator<<(std::ostream& out,const StackTrace::Stack_T& stack) {

    for (const auto & frame : stack)
    {
        out << "[" << frame.m_pos << "] " << frame.m_addr << " <> " << frame.m_name.toStdString() << std::endl;
    }

    return out;
}

}}
