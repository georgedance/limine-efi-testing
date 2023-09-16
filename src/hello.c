#ifndef GO_AWAY_CLANGD
#include <efi/efi.h>
#else
#include <efi.h>
#endif

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    (void)ImageHandle;
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Hello world\r\n");
    return EFI_SUCCESS;
}
