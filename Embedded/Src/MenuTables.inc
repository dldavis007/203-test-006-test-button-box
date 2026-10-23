/* Default and runtime menu tables. Included only by Subroutines.c
 * at the original definition location. Each item retains its original slot.
 * Text rows: 0 = title, 1..11 = entries. Position/variable/action slots 1..11
 * refer to those entry rows. A value column of zero suppresses variable text.
 */
const struct MenuStruct defaultMenuc[MenuSize] = {

    /* Menu [00]: BUTTON BOX */
    {
        /* Menu path */
        {0, 0, 0, 0},

        /* Display text (20 columns) */
        {
            "     BUTTON BOX     ",   /*  0: title */
            " SETTINGS           ",   /*  1: SETTINGS */
            " DIAGNOSTICS        ",   /*  2: DIAGNOSTICS */
            " CAMERAS            ",   /*  3: CAMERAS */
            " STATUS             ",   /*  4: STATUS */
            " AUXILIARY EQUIP    ",   /*  5: AUXILIARY EQUIP */
            " DEFAULTS           ",   /*  6: DEFAULTS */
            " EXIT               ",   /*  7: EXIT */
            "                    ",   /*  8: unused */
            "                    ",   /*  9: unused */
            "                    ",   /* 10: unused */
            "                    "    /* 11: unused */
        },

        /* Value columns (zero = no variable) */
        {
            0,                        /*  1: SETTINGS */
            0,                        /*  2: DIAGNOSTICS */
            0,                        /*  3: CAMERAS */
            0,                        /*  4: STATUS */
            0,                        /*  5: AUXILIARY EQUIP */
            0,                        /*  6: DEFAULTS */
            0,                        /*  7: EXIT */
            0,                        /*  8: unused */
            0,                        /*  9: unused */
            0,                        /* 10: unused */
            0                         /* 11: unused */
        },

        /* Variables */
        {
            NullVar,                  /*  1: SETTINGS */
            NullVar,                  /*  2: DIAGNOSTICS */
            NullVar,                  /*  3: CAMERAS */
            NullVar,                  /*  4: STATUS */
            NullVar,                  /*  5: AUXILIARY EQUIP */
            NullVar,                  /*  6: DEFAULTS */
            NullVar,                  /*  7: EXIT */
            NullVar,                  /*  8: unused */
            NullVar,                  /*  9: unused */
            NullVar,                  /* 10: unused */
            NullVar                   /* 11: unused */
        },

        /* Actions */
        {
            &NullFunction,            /*  1: SETTINGS */
            &NullFunction,            /*  2: DIAGNOSTICS */
            &NullFunction,            /*  3: CAMERAS */
            &NullFunction,            /*  4: STATUS */
            &NullFunction,            /*  5: AUXILIARY EQUIP */
            &ExitMenu,                /*  6: DEFAULTS */
            &ExitMenu,                /*  7: EXIT */
            &ExitMenu,                /*  8: unused */
            &ExitMenu,                /*  9: unused */
            &ExitMenu,                /* 10: unused */
            &ExitMenu                 /* 11: unused */
        }
    },

    /* Menu [01]: SETTINGS MENU */
    {
        /* Menu path */
        {0, 0, 0, 1},

        /* Display text (20 columns) */
        {
            "    SETTINGS MENU   ",   /*  0: title */
            " MESSAGE ROW        ",   /*  1: MESSAGE ROW */
            " MESSAGE COL        ",   /*  2: MESSAGE COL */
            " F1 CAM MODE        ",   /*  3: F1 CAM MODE */
            " F2 CTRL MODE       ",   /*  4: F2 CTRL MODE */
            " EXIT               ",   /*  5: EXIT */
            "                    ",   /*  6: unused */
            "                    ",   /*  7: unused */
            "                    ",   /*  8: unused */
            "                    ",   /*  9: unused */
            "                    ",   /* 10: unused */
            "                    "    /* 11: unused */
        },

        /* Value columns (zero = no variable) */
        {
            15,                       /*  1: MESSAGE ROW */
            15,                       /*  2: MESSAGE COL */
            15,                       /*  3: F1 CAM MODE */
            15,                       /*  4: F2 CTRL MODE */
            0,                        /*  5: EXIT */
            0,                        /*  6: unused */
            0,                        /*  7: unused */
            0,                        /*  8: unused */
            0,                        /*  9: unused */
            0,                        /* 10: unused */
            0                         /* 11: unused */
        },

        /* Variables */
        {
            MessageRow,               /*  1: MESSAGE ROW */
            MessageCol,               /*  2: MESSAGE COL */
            F1Enable,                 /*  3: F1 CAM MODE */
            F2Enable,                 /*  4: F2 CTRL MODE */
            NullVar,                  /*  5: EXIT */
            NullVar,                  /*  6: unused */
            NullVar,                  /*  7: unused */
            NullVar,                  /*  8: unused */
            NullVar,                  /*  9: unused */
            NullVar,                  /* 10: unused */
            NullVar                   /* 11: unused */
        },

        /* Actions */
        {
            &Var1to13Funct,           /*  1: MESSAGE ROW */
            &Var1to30Funct,           /*  2: MESSAGE COL */
            &VarOffOnFunct,           /*  3: F1 CAM MODE */
            &VarOffOnFunct,           /*  4: F2 CTRL MODE */
            &ExitMenu,                /*  5: EXIT */
            &NullFunction,            /*  6: unused */
            &NullFunction,            /*  7: unused */
            &NullFunction,            /*  8: unused */
            &NullFunction,            /*  9: unused */
            &NullFunction,            /* 10: unused */
            &NullFunction             /* 11: unused */
        }
    },

    /* Menu [02]: DIAGNOSTIC MENU */
    {
        /* Menu path */
        {0, 0, 0, 2},

        /* Display text (20 columns) */
        {
            "   DIAGNOSTIC MENU  ",   /*  0: title */
            " EXIT               ",   /*  1: EXIT */
            "                    ",   /*  2: unused */
            "                    ",   /*  3: unused */
            "                    ",   /*  4: unused */
            "                    ",   /*  5: unused */
            "                    ",   /*  6: unused */
            "                    ",   /*  7: unused */
            "                    ",   /*  8: unused */
            "                    ",   /*  9: unused */
            "                    ",   /* 10: unused */
            "                    "    /* 11: unused */
        },

        /* Value columns (zero = no variable) */
        {
            0,                        /*  1: EXIT */
            0,                        /*  2: unused */
            0,                        /*  3: unused */
            0,                        /*  4: unused */
            0,                        /*  5: unused */
            0,                        /*  6: unused */
            0,                        /*  7: unused */
            0,                        /*  8: unused */
            0,                        /*  9: unused */
            0,                        /* 10: unused */
            0                         /* 11: unused */
        },

        /* Variables */
        {
            NullVar,                  /*  1: EXIT */
            NullVar,                  /*  2: unused */
            NullVar,                  /*  3: unused */
            NullVar,                  /*  4: unused */
            NullVar,                  /*  5: unused */
            NullVar,                  /*  6: unused */
            NullVar,                  /*  7: unused */
            NullVar,                  /*  8: unused */
            NullVar,                  /*  9: unused */
            NullVar,                  /* 10: unused */
            NullVar                   /* 11: unused */
        },

        /* Actions */
        {
            &ExitMenu,                /*  1: EXIT */
            &NullFunction,            /*  2: unused */
            &NullFunction,            /*  3: unused */
            &NullFunction,            /*  4: unused */
            &NullFunction,            /*  5: unused */
            &NullFunction,            /*  6: unused */
            &NullFunction,            /*  7: unused */
            &NullFunction,            /*  8: unused */
            &NullFunction,            /*  9: unused */
            &NullFunction,            /* 10: unused */
            &NullFunction             /* 11: unused */
        }
    },

    /* Menu [03]: CAMERAS */
    {
        /* Menu path */
        {0, 0, 0, 3},

        /* Display text (20 columns) */
        {
            "      CAMERAS       ",   /*  0: title */
            " FIND CAMERAS       ",   /*  1: FIND CAMERAS */
            " EXIT               ",   /*  2: EXIT */
            "                    ",   /*  3: unused */
            "                    ",   /*  4: unused */
            "                    ",   /*  5: unused */
            "                    ",   /*  6: unused */
            "                    ",   /*  7: unused */
            "                    ",   /*  8: unused */
            "                    ",   /*  9: unused */
            "                    ",   /* 10: unused */
            "                    "    /* 11: unused */
        },

        /* Value columns (zero = no variable) */
        {
            0,                        /*  1: FIND CAMERAS */
            0,                        /*  2: EXIT */
            0,                        /*  3: unused */
            0,                        /*  4: unused */
            0,                        /*  5: unused */
            0,                        /*  6: unused */
            0,                        /*  7: unused */
            0,                        /*  8: unused */
            0,                        /*  9: unused */
            0,                        /* 10: unused */
            0                         /* 11: unused */
        },

        /* Variables */
        {
            NullVar,                  /*  1: FIND CAMERAS */
            NullVar,                  /*  2: EXIT */
            NullVar,                  /*  3: unused */
            NullVar,                  /*  4: unused */
            NullVar,                  /*  5: unused */
            NullVar,                  /*  6: unused */
            NullVar,                  /*  7: unused */
            NullVar,                  /*  8: unused */
            NullVar,                  /*  9: unused */
            NullVar,                  /* 10: unused */
            NullVar                   /* 11: unused */
        },

        /* Actions */
        {
            &findCameras,             /*  1: FIND CAMERAS */
            &ExitMenu,                /*  2: EXIT */
            &ExitMenu,                /*  3: unused */
            &ExitMenu,                /*  4: unused */
            &ExitMenu,                /*  5: unused */
            &ExitMenu,                /*  6: unused */
            &ExitMenu,                /*  7: unused */
            &ExitMenu,                /*  8: unused */
            &ExitMenu,                /*  9: unused */
            &ExitMenu,                /* 10: unused */
            &ExitMenu                 /* 11: unused */
        }
    },

    /* Menu [04]: STATUS MENU */
    {
        /* Menu path */
        {0, 0, 0, 4},

        /* Display text (20 columns) */
        {
            "     STATUS MENU    ",   /*  0: title */
            " SOFTWARE REV       ",   /*  1: SOFTWARE REV */
            " SERIAL NUM         ",   /*  2: SERIAL NUM */
            " EXIT               ",   /*  3: EXIT */
            "                    ",   /*  4: unused */
            "                    ",   /*  5: unused */
            "                    ",   /*  6: unused */
            "                    ",   /*  7: unused */
            "                    ",   /*  8: unused */
            "                    ",   /*  9: unused */
            "                    ",   /* 10: unused */
            "                    "    /* 11: unused */
        },

        /* Value columns (zero = no variable) */
        {
            16,                       /*  1: SOFTWARE REV */
            15,                       /*  2: SERIAL NUM */
            0,                        /*  3: EXIT */
            0,                        /*  4: unused */
            0,                        /*  5: unused */
            0,                        /*  6: unused */
            0,                        /*  7: unused */
            0,                        /*  8: unused */
            0,                        /*  9: unused */
            0,                        /* 10: unused */
            0                         /* 11: unused */
        },

        /* Variables */
        {
            Rev,                      /*  1: SOFTWARE REV */
            SerialNum,                /*  2: SERIAL NUM */
            NullVar,                  /*  3: EXIT */
            NullVar,                  /*  4: unused */
            NullVar,                  /*  5: unused */
            NullVar,                  /*  6: unused */
            NullVar,                  /*  7: unused */
            NullVar,                  /*  8: unused */
            NullVar,                  /*  9: unused */
            NullVar,                  /* 10: unused */
            NullVar                   /* 11: unused */
        },

        /* Actions */
        {
            &NullFunction,            /*  1: SOFTWARE REV */
            &setSerial,               /*  2: SERIAL NUM */
            &ExitMenu,                /*  3: EXIT */
            &NullFunction,            /*  4: unused */
            &NullFunction,            /*  5: unused */
            &NullFunction,            /*  6: unused */
            &NullFunction,            /*  7: unused */
            &NullFunction,            /*  8: unused */
            &NullFunction,            /*  9: unused */
            &NullFunction,            /* 10: unused */
            &NullFunction             /* 11: unused */
        }
    },

    /* Menu [05]: AUXILIARY EQUIP */
    {
        /* Menu path */
        {0, 0, 0, 5},

        /* Display text (20 columns) */
        {
            "   AUXILIARY EQUIP  ",   /*  0: title */
            " EXIT               ",   /*  1: EXIT */
            "                    ",   /*  2: unused */
            "                    ",   /*  3: unused */
            "                    ",   /*  4: unused */
            "                    ",   /*  5: unused */
            "                    ",   /*  6: unused */
            "                    ",   /*  7: unused */
            "                    ",   /*  8: unused */
            "                    ",   /*  9: unused */
            "                    ",   /* 10: unused */
            "                    "    /* 11: unused */
        },

        /* Value columns (zero = no variable) */
        {
            0,                        /*  1: EXIT */
            0,                        /*  2: unused */
            0,                        /*  3: unused */
            0,                        /*  4: unused */
            0,                        /*  5: unused */
            0,                        /*  6: unused */
            0,                        /*  7: unused */
            0,                        /*  8: unused */
            0,                        /*  9: unused */
            0,                        /* 10: unused */
            0                         /* 11: unused */
        },

        /* Variables */
        {
            NullVar,                  /*  1: EXIT */
            NullVar,                  /*  2: unused */
            NullVar,                  /*  3: unused */
            NullVar,                  /*  4: unused */
            NullVar,                  /*  5: unused */
            NullVar,                  /*  6: unused */
            NullVar,                  /*  7: unused */
            NullVar,                  /*  8: unused */
            NullVar,                  /*  9: unused */
            NullVar,                  /* 10: unused */
            NullVar                   /* 11: unused */
        },

        /* Actions */
        {
            &ExitMenu,                /*  1: EXIT */
            &ExitMenu,                /*  2: unused */
            &ExitMenu,                /*  3: unused */
            &ExitMenu,                /*  4: unused */
            &ExitMenu,                /*  5: unused */
            &ExitMenu,                /*  6: unused */
            &ExitMenu,                /*  7: unused */
            &ExitMenu,                /*  8: unused */
            &ExitMenu,                /*  9: unused */
            &ExitMenu,                /* 10: unused */
            &ExitMenu                 /* 11: unused */
        }
    },

    /* Menu [06]: DEFAULTS */
    {
        /* Menu path */
        {0, 0, 0, 6},

        /* Display text (20 columns) */
        {
            "     DEFAULTS       ",   /*  0: title */
            " EXIT               ",   /*  1: EXIT */
            " RESTORE DEFAULTS   ",   /*  2: RESTORE DEFAULTS */
            "                    ",   /*  3: unused */
            "      WARNING       ",   /*  4: WARNING */
            "YOU WILL LOSE ALL   ",   /*  5: YOU WILL LOSE ALL */
            "OF THE CURRENT      ",   /*  6: OF THE CURRENT */
            "SETTINGS WHEN       ",   /*  7: SETTINGS WHEN */
            "RESTORING DEFAULTS  ",   /*  8: RESTORING DEFAULTS */
            "                    ",   /*  9: unused */
            "                    ",   /* 10: unused */
            "                    "    /* 11: unused */
        },

        /* Value columns (zero = no variable) */
        {
            0,                        /*  1: EXIT */
            0,                        /*  2: RESTORE DEFAULTS */
            0,                        /*  3: unused */
            0,                        /*  4: WARNING */
            0,                        /*  5: YOU WILL LOSE ALL */
            0,                        /*  6: OF THE CURRENT */
            0,                        /*  7: SETTINGS WHEN */
            0,                        /*  8: RESTORING DEFAULTS */
            0,                        /*  9: unused */
            0,                        /* 10: unused */
            0                         /* 11: unused */
        },

        /* Variables */
        {
            NullVar,                  /*  1: EXIT */
            NullVar,                  /*  2: RESTORE DEFAULTS */
            NullVar,                  /*  3: unused */
            NullVar,                  /*  4: WARNING */
            NullVar,                  /*  5: YOU WILL LOSE ALL */
            NullVar,                  /*  6: OF THE CURRENT */
            NullVar,                  /*  7: SETTINGS WHEN */
            NullVar,                  /*  8: RESTORING DEFAULTS */
            NullVar,                  /*  9: unused */
            NullVar,                  /* 10: unused */
            NullVar                   /* 11: unused */
        },

        /* Actions */
        {
            &ExitMenu,                /*  1: EXIT */
            &RestoreDefaults,         /*  2: RESTORE DEFAULTS */
            &NullFunction,            /*  3: unused */
            &NullFunction,            /*  4: WARNING */
            &NullFunction,            /*  5: YOU WILL LOSE ALL */
            &NullFunction,            /*  6: OF THE CURRENT */
            &NullFunction,            /*  7: SETTINGS WHEN */
            &NullFunction,            /*  8: RESTORING DEFAULTS */
            &NullFunction,            /*  9: unused */
            &NullFunction,            /* 10: unused */
            &NullFunction             /* 11: unused */
        }
    }
};


struct MenuStruct Menuc[MenuSize];

/* Only the first stack entry is initialized; remaining entries stay zero. */
const struct MenuStack defaultMenuStackc[MenuStackSize] = {
    {
        {0, 0, 0, 0},    /* Root menu path */
        1,                /* Initial cursor row */
        0                 /* First visible entry offset */
    }
};
struct MenuStack MenuStackc[MenuStackSize];
