// Display Green Button
        hw.display.SetCursor(0, 42); 
        if (grn_pressed) hw.display.WriteString("GB: DOWN", Font_7x10, true);
        else             hw.display.WriteString("GB: UP  ", Font_7x10, true);

        // Display Red Button
        hw.display.SetCursor(0, 54); 
        if (red_pressed) hw.display.WriteString("RB: DOWN", Font_7x10, true);
        else             hw.display.WriteString("RB: UP  ", Font_7x10, true);