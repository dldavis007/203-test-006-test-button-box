/* Default and runtime menu tables. Included only by Subroutines.c
   at the original definition location to preserve scope and storage order. */
const struct MenuStruct defaultMenuc[MenuSize] = {
    {
        {0, 0, 0, 0},
        {"     BUTTON BOX     ", " SETTINGS           ", " DIAGNOSTICS        ", " CAMERAS            ", " STATUS             ", " AUXILIARY EQUIP    ", " DEFAULTS           ", " EXIT               ", "                    ", "                    ", "                    ", "                    "},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar},
        {&NullFunction, &NullFunction, &NullFunction, &NullFunction, &NullFunction, &ExitMenu, &ExitMenu, &ExitMenu, &ExitMenu, &ExitMenu, &ExitMenu}
    },
    {
        {0, 0, 0, 1},
        {"    SETTINGS MENU   ", " MESSAGE ROW        ", " MESSAGE COL        ", " F1                 ", " F2                 ", " EXIT               ", "                    ", "                    ", "                    ", "                    ", "                    ", "                    "},
        {15, 15, 15, 15, 0, 0, 0, 0, 0, 0, 0},
        {MessageRow, MessageCol, F1Enable, F2Enable, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar},
        {&Var1to13Funct, &Var1to30Funct, &ToggleF1Enable, &ToggleF2Enable, &ExitMenu, &NullFunction, &NullFunction, &NullFunction, &NullFunction, &NullFunction, &NullFunction}
    },
    {
        {0, 0, 0, 2},
        {"   DIAGNOSTIC MENU  ", " EXIT               ", "                    ", "                    ", "                    ", "                    ", "                    ", "                    ", "                    ", "                    ", "                    ", "                    "},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar},
        {&ExitMenu, &NullFunction, &NullFunction, &NullFunction, &NullFunction, &NullFunction, &NullFunction, &NullFunction, &NullFunction, &NullFunction, &NullFunction}
    },
    {
        {0, 0, 0, 3},
        {"      CAMERAS       ", " FIND CAMERAS       ", " EXIT               ", "                    ", "                    ", "                    ", "                    ", "                    ", "                    ", "                    ", "                    ", "                    "},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar},
        {&findCameras, &ExitMenu, &ExitMenu, &ExitMenu, &ExitMenu, &ExitMenu, &ExitMenu, &ExitMenu, &ExitMenu, &ExitMenu, &ExitMenu}
    },
    {
        {0, 0, 0, 4},
        {"     STATUS MENU    ", " SOFTWARE REV       ", " SERIAL NUM         ", " EXIT               ", "                    ", "                    ", "                    ", "                    ", "                    ", "                    ", "                    ", "                    "},
        {16, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {Rev, SerialNum, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar},
        {&NullFunction, &setSerial, &ExitMenu, &NullFunction, &NullFunction, &NullFunction, &NullFunction, &NullFunction, &NullFunction, &NullFunction, &NullFunction}
    },
    {
        {0, 0, 0, 5},
        {"   AUXILIARY EQUIP  ", " EXIT               ", "                    ", "                    ", "                    ", "                    ", "                    ", "                    ", "                    ", "                    ", "                    ", "                    "},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar},
        {&ExitMenu, &ExitMenu, &ExitMenu, &ExitMenu, &ExitMenu, &ExitMenu, &ExitMenu, &ExitMenu, &ExitMenu, &ExitMenu, &ExitMenu}
    },
    {
        {0, 0, 0, 6},
        {"     DEFAULTS       ", " EXIT               ", " RESTORE DEFAULTS   ", "                    ", "      WARNING       ", "YOU WILL LOSE ALL   ", "OF THE CURRENT      ", "SETTINGS WHEN       ", "RESTORING DEFAULTS  ", "                    ", "                    ", "                    "},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar, NullVar},
        {&ExitMenu, &RestoreDefaults, &NullFunction, &NullFunction, &NullFunction, &NullFunction, &NullFunction, &NullFunction, &NullFunction, &NullFunction, &NullFunction}
    }
};

struct MenuStruct Menuc[MenuSize];

const struct MenuStack defaultMenuStackc[MenuStackSize] = {
    {
        {0, 0, 0, 0},
        1,
        0
    }
};
struct MenuStack MenuStackc[MenuStackSize];
