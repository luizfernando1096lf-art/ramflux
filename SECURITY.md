# Security Policy

## Supported Versions

| Version | Supported          |
|---------|--------------------|
| 2.44.x  | :white_check_mark: |
| 2.43.x  | :white_check_mark: |
| 2.42.x  | :white_check_mark: |
| < 2.42  | :x:                |

## Reporting a Vulnerability

RAMFlux performs low-level memory operations using NTAPI and requires elevated privileges. Security is taken seriously.

To report a security vulnerability:

1. **Do not** open a public GitHub issue
2. Send an email to the maintainer via the GitHub contact information
3. Include a detailed description of the vulnerability, reproduction steps, and affected version

You should receive a response within 48 hours. If the vulnerability is accepted, a fix will be prioritized and released in the next patch or minor version depending on severity.

## Security Considerations

### Administrator Privileges
RAMFlux uses a separate helper process (`RAMFluxHelper.exe`) that runs with administrator privileges. This process is the only component that performs NTAPI-level memory operations. The main UI process runs at the user's privilege level.

### Protected Processes
The following processes are **never** touched by RAMFlux:
- System critical processes (CSRSS, Winlogon, Services, etc.)
- Antivirus and security software
- Any process marked as a critical Windows process

### Data Collection
RAMFlux does **not** collect or transmit any user data, telemetry, or personally identifiable information. All data processing is local.

### Code Integrity
- All NTAPI calls are isolated in the `src/ntapi/` module
- Memory operations are validated before execution
- The privileged helper process validates all requests from the main process
- Safety checks are performed before every optimization operation
- **v2.5.2**: Full code audit completed — 18 source files reviewed for injection, buffer overflow, race conditions, null pointers, and resource leaks. Zero findings.
