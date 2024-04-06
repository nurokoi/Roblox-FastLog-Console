# Roblox-FastLog-Console

Shows internal FastLog logs for **Roblox Studio & Roblox Player**. Utilizes the Win32 API to hook into [`OutputDebugString`](https://learn.microsoft.com/en-us/windows/win32/api/debugapi/nf-debugapi-outputdebugstringa) calls, revealing the internal logs typically used by Roblox engineers for diagnostics within Visual Studio.

---

## Usage

### Compilation
- **Compile the binary with g++:**  
g++ -o flog_monitor main.cpp

> **Note:** Ensure you have g++ installed and your environment is set up for Windows development.

### Running & Usage
- **Run `flog_monitor` with the names of the Roblox processes you wish to monitor:**  
1. Studio & Client ``flog_monitor RobloxPlayerBeta.exe RobloxStudioBeta.exe``
2. Studio ``flog_monitor RobloxStudioBeta.exe`` 
3. Client ``flog_monitor RobloxPlayerBeta.exe`` 

### For those who cannot compile or dont know
- Use the latest prebuilt binary in [releases](https://github.com/nurokoi/roblox-fastlog-console/releases) to get a copy of `flog_monitor` binary.

> **Note:** This tool is specifically designed for the Win32 API and will not function on non-Windows platforms.

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## About `Roblox-FastLog-Console`

`Roblox-FastLog-Console` is a diagnostic tool that bridges the gap for Roblox developers by giving access to the internal console logs utilized by Roblox engineers. These logs, often hidden from the developers play a role in:

- Advanced diagnostics
- Troubleshooting within professional development environments

**Key Features:**

- **Broad Log Access:** Not just limited to FastLog calls, `Roblox-FastLog-Console` also captures internal Roblox API behaviour, standard console logs, metrics, and more.
- **Error Identification:** Identifies runtime errors that Roblox doesn't show for potential crash causes, bugs in the Roblox software, compiler errors, and more.

*Remember, this tool is for **diagnostic purposes only** and should be used responsibly and ethically.*
