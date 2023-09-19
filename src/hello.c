#include "../limine-efi/inc/efi.h"

EFI_SYSTEM_TABLE        *ST;
#define gST              ST
EFI_BOOT_SERVICES       *BS;
#define gBS              BS
EFI_RUNTIME_SERVICES    *RT;
#define gRT              RT

SIMPLE_INPUT_INTERFACE          *efi_conin;
#define stdin                    efi_conin
SIMPLE_TEXT_OUTPUT_INTERFACE    *efi_conout;
#define stdout                   efi_conout
SIMPLE_TEXT_OUTPUT_INTERFACE    *efi_stderr;
#define stderr                   efi_stderr

EFI_STATUS status;

#define uefi_call(x) \
    status = x; \
    if(status != EFI_SUCCESS) { \
        return status; \
    }

EFI_STATUS clear(SIMPLE_TEXT_OUTPUT_INTERFACE *stoi) {
    return stoi->ClearScreen(stoi);
}

EFI_STATUS print(SIMPLE_TEXT_OUTPUT_INTERFACE *stoi, wchar_t *string) {
    return stoi->OutputString(stoi, string);
}

/*
 *  itoa() function taken from https://stackoverflow.com/a/12386915
 */
int itoa(int value, wchar_t *sp, int radix)
{
    char tmp[16];// be careful with the length of the buffer
    char *tp = tmp;
    int i;
    unsigned v;

    int sign = (radix == 10 && value < 0);
    if (sign)
        v = -value;
    else
        v = (unsigned)value;

    while (v || tp == tmp)
    {
        i = v % radix;
        v /= radix;
        if (i < 10)
          *tp++ = i+'0';
        else
          *tp++ = i + 'a' - 10;
    }

    int len = tp - tmp;

    if (sign)
    {
        *sp++ = '-';
        len++;
    }

    while (tp > tmp)
        *sp++ = *--tp;

    return len;
}

EFI_STATUS hundred_nsleep(UINT64 hundred_nanoseconds) {
    EFI_EVENT timer_event;
    EFI_EVENT wait_list[1];
    UINT64 index;

    uefi_call(gBS->CreateEvent(EVT_TIMER, TPL_NOTIFY, NULL, NULL, &timer_event));
    uefi_call(gBS->SetTimer(timer_event, TimerRelative, hundred_nanoseconds));
    wait_list[0] = timer_event;
    uefi_call(gBS->WaitForEvent(1, wait_list, &index));
    uefi_call(gBS->CloseEvent(timer_event));

    return EFI_SUCCESS;
}

EFI_STATUS usleep(UINT64 microseconds) {
    return hundred_nsleep(microseconds * 10);
}

EFI_STATUS msleep(UINT64 milliseconds) {
    return usleep(milliseconds * 1000);
}

EFI_STATUS sleep(UINT64 seconds) {
    return msleep(seconds * 1000);
}

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    (void)ImageHandle;

    gST = SystemTable;
    gBS = gST->BootServices;
    gRT = gST->RuntimeServices;

    stdin  = gST->ConIn;
    stdout = gST->ConOut;
    stderr = gST->StdErr;

    //uefi_call(print(stdout, L"Hello World\r\n"));

    EFI_TIME *time;
    EFI_TIME_CAPABILITIES *time_cap;

    uefi_call(gBS->AllocatePool(EfiBootServicesData, sizeof(EFI_TIME), (void **) &time));

    uefi_call(gBS->AllocatePool(EfiBootServicesData, sizeof(EFI_TIME_CAPABILITIES), (void **) &time_cap));

    for(int i = 0; i < 10; i++) {
        uefi_call(clear(stdout));

        uefi_call(gRT->GetTime(time, time_cap));

        wchar_t hour[4];
        itoa(time->Hour, hour, 10);
        wchar_t minute[4];
        itoa(time->Minute, minute, 10);
        wchar_t second[4];
        itoa(time->Second, second, 10);

        print(stdout, L"Time");
        if(time->TimeZone == 2047) {
            print(stdout, L" (UTC+0)");
        }
        print(stdout, L": ");
        print(stdout, hour);
        print(stdout, L":");
        print(stdout, minute);
        print(stdout, L":");
        print(stdout, second);
        print(stdout, L"\r\n");

        // TODO: FIXME: XXX:
        // When rolling over the minute, the 9 from 59 is kept in the 1's column for the seconds display.
        //   i'd imagine this also happend when rolling over the hour, etc

        sleep(1);
    }

    uefi_call(gBS->FreePool(time));
    uefi_call(gBS->FreePool(time_cap));

    return EFI_SUCCESS;
}
