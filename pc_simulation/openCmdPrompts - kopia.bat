start powershell -NoExit -Command "& { 
    $hwnd = Get-Process -Id $pid | ForEach-Object { $_.MainWindowHandle };
    Add-Type @'
        using System;
        using System.Runtime.InteropServices;
        public class WinAPI {
            [DllImport(\"user32.dll\")]
            [return: MarshalAs(UnmanagedType.Bool)]
            public static extern bool MoveWindow(IntPtr hWnd, int X, int Y, int nWidth, int nHeight, bool bRepaint);
        }
'@;
    [WinAPI]::MoveWindow($hwnd, 100, 100, 800, 600, $true);
    cd 'G:\_git\__ESP\esp-wifi_env_sensor\pc_simulation'
}"
