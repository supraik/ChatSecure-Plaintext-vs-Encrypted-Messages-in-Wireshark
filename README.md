# 🔐 Encrypted Chat Application with Wireshark Packet Analysis

## 📋 Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Installation](#installation)
- [Quick Start](#quick-start)
- [Wireshark Setup & Analysis](#wireshark-setup--analysis)
- [How It Works](#how-it-works)
- [Project Structure](#project-structure)
- [Testing Scenarios](#testing-scenarios)
- [Troubleshooting](#troubleshooting)
- [Security Notes](#security-notes)
- [Learning Objectives](#learning-objectives)

---

## 🎯 Overview

This project demonstrates:
- **Network packet visibility** in Wireshark
- **Why encryption matters** for data security
- **Client-server architecture** with real-time communication
- **Symmetric encryption** (AES-256) implementation

Perfect for learning about network security, socket programming, and ethical hacking concepts.

---

## ✨ Features

### 🔓 Plaintext Mode
- Messages sent in readable JSON format
- Fully visible in Wireshark packet capture
- Demonstrates vulnerability of unencrypted communication

### 🔐 Encrypted Mode
- AES-256 symmetric encryption
- Messages encrypted before transmission
- Unreadable in Wireshark (shows base64-encoded gibberish)

### 💬 Chat Features
- Multi-client support (2+ users simultaneously)
- Real-time bidirectional communication
- System notifications (user join/leave)
- Username display
- Clean Tkinter GUI

### 🎨 User Interface
- Easy-to-use checkbox to toggle encryption
- Visual status indicator (Red = Plaintext, Green = Encrypted)
- Color-coded messages (Green = You, Black = Others, Blue = System)
- Scrollable chat history

---

## 📥 Installation

### Prerequisites
- Python 3.7 or higher
- pip (Python package manager)

### Step 1: Install Python Dependencies

```bash
pip install cryptography
```

**Note:** `socket`, `threading`, and `tkinter` are included in Python's standard library.

### Step 2: Verify Tkinter

Tkinter usually comes with Python. Test it:

```bash
python -m tkinter
```

If a small window appears, you're ready! ✅

**If Tkinter is missing:**
- **Ubuntu/Debian:** `sudo apt-get install python3-tk`
- **macOS:** Should be pre-installed with Python
- **Windows:** Reinstall Python with "tcl/tk and IDLE" option checked

### Step 3: Install Wireshark

Download from: [https://www.wireshark.org/download.html](https://www.wireshark.org/download.html)

**Platform-specific notes:**
- **Windows:** Install Npcap when prompted (required for capture)
- **Linux:** Add yourself to wireshark group: `sudo usermod -aG wireshark $USER` (then log out/in)
- **macOS:** First run requires right-click → Open to bypass security

---

## 🚀 Quick Start

### Step 1: Download the Project Files

Create a project directory and save these files:
- `server.py` - The chat server
- `client.py` - The chat client with GUI

### Step 2: Start the Server

Open a terminal and run:

```bash
python server.py
```

**Expected output:**
```
[LISTENING] Server is listening on 127.0.0.1:5555
[INFO] Waiting for connections...
```

✅ Server is now running and waiting for clients!

### Step 3: Start First Client (Alice)

Open a **new terminal** and run:

```bash
python client.py
```

1. A prompt appears asking for username
2. Enter: `Alice`
3. Click OK
4. Chat window opens with title "Chat Client - Alice"

### Step 4: Start Second Client (Bob)

Open **another new terminal** and run:

```bash
python client.py
```

1. Enter username: `Bob`
2. Click OK
3. Second chat window opens

### Step 5: Test Communication

**In Alice's window:**
- Type: "Hello Bob!"
- Click "Send Message"

**In Bob's window:**
- You should see: "Alice: Hello Bob!"

**In Alice's window:**
- You should see: "You: Hello Bob!" (in green)

🎉 **Success! Your chat is working!**

---

## 🦈 Wireshark Setup & Analysis

### Part 1: Install and Launch Wireshark

1. **Open Wireshark** (may require admin/sudo on first run)
2. You'll see a list of network interfaces

### Part 2: Select Loopback Interface

Find and **double-click** one of these:
- **Windows:** "Adapter for loopback traffic capture" or "Npcap Loopback Adapter"
- **macOS:** "Loopback: lo0" or just "lo0"
- **Linux:** "Loopback: lo" or just "lo"

**Visual tip:** Look for the circular arrow icon 🔄

Packets will immediately start scrolling. Don't worry, we'll filter them next!

### Part 3: Apply Display Filter

In the green filter bar at the top, type:

```
tcp.port == 5555
```

Press **Enter**

Now you'll only see chat application traffic! 🎯

### Part 4: Capture Plaintext Messages

#### 4.1 Ensure Encryption is OFF

In **both** Alice and Bob's chat windows:
- **Uncheck** "Enable Encryption (AES)" checkbox
- Status should show: **"Status: Plain Text Mode"** (in red)

#### 4.2 Send a Test Message

From Alice's chat window, type:
```
My secret password is admin123
```
Click "Send Message"

#### 4.3 Find and Inspect the Packet

1. **In Wireshark:** Look for the newest packet (just appeared)
2. **Click** on the packet to select it
3. **In the middle pane,** expand these sections:
   - ▸ **Transmission Control Protocol**
   - ▸ **TCP segment data**
4. **In the bottom pane** (Packet Bytes), look at the right side (ASCII column)

#### 4.4 What You'll See ✅

```
{"type": "plaintext", "message": "My secret password is admin123", "username": "Alice"}
```

🚨 **The password is completely visible!** Anyone capturing packets can read it.

### Part 5: Capture Encrypted Messages

#### 5.1 Enable Encryption

In **both** Alice and Bob's chat windows:
- **Check** the "Enable Encryption (AES)" checkbox
- Status changes to: **"Status: Encrypted Mode (AES)"** (in green)

#### 5.2 Send the Same Message

From Alice's chat window, type the **exact same message**:
```
My secret password is admin123
```
Click "Send Message"

#### 5.3 Inspect the Encrypted Packet

1. **In Wireshark:** Find the new packet
2. Click on it and expand **TCP segment data** (same steps as before)
3. Look at the bottom pane ASCII view

#### 5.4 What You'll See ✅

```
{"type": "encrypted", "data": "Z0FBQUFBQm5kX3l2TE5QT... [gibberish continues] ...", "username": "Alice"}
```

🔒 **You CANNOT read the password anymore!** The "data" field contains encrypted bytes encoded as base64.

### Part 6: Follow TCP Stream (Bonus)

For easier viewing:

1. **Right-click** on any packet
2. Select **"Follow"** → **"TCP Stream"**
3. A new window opens showing the entire conversation
   - **Red text** = Client to server
   - **Blue text** = Server to client

Compare plaintext vs encrypted messages in one view!

### Part 7: Useful Wireshark Filters

| Filter | Purpose |
|--------|---------|
| `tcp.port == 5555` | Show only chat traffic |
| `tcp.port == 5555 and frame contains "plaintext"` | Show only unencrypted messages |
| `tcp.port == 5555 and frame contains "encrypted"` | Show only encrypted messages |
| `tcp.flags.push == 1` | Show packets containing data |
| `ip.addr == 127.0.0.1` | Show localhost traffic only |

---

## 🔧 How It Works

### Architecture Overview

```
┌─────────┐         ┌─────────┐         ┌─────────┐
│ Client  │◄───────►│ Server  │◄───────►│ Client  │
│ (Alice) │   TCP   │  :5555  │   TCP   │  (Bob)  │
└─────────┘         └─────────┘         └─────────┘
```

### Plaintext Mode Flow

```
Alice types: "Hello"
    ↓
JSON: {"type": "plaintext", "message": "Hello"}
    ↓
Sent over TCP socket to server (127.0.0.1:5555)
    ↓
Server receives and broadcasts to all clients except Alice
    ↓
Bob receives and displays: "Alice: Hello"
```

**Wireshark sees:** Readable JSON with "Hello" visible

### Encrypted Mode Flow

```
Alice types: "Hello"
    ↓
Encrypt with AES-256: b'gAAAAABn...[binary data]'
    ↓
Base64 encode: "Z0FBQUFBQm4..."
    ↓
JSON: {"type": "encrypted", "data": "Z0FBQUFBQm4..."}
    ↓
Sent over TCP socket to server
    ↓
Server broadcasts to Bob
    ↓
Bob receives, base64 decodes, decrypts with shared key
    ↓
Bob displays: "Alice (encrypted): Hello"
```

**Wireshark sees:** JSON with unreadable base64 gibberish

### Encryption Details

- **Algorithm:** AES-256 via Fernet (symmetric encryption)
- **Key Generation:** SHA-256 hash of shared secret
- **Shared Secret:** `"my_super_secret_key_12345"` (hardcoded for demo)
- **Library:** Python `cryptography` package
- **Mode:** Fernet uses AES-128 in CBC mode with HMAC for authentication

**Security note:** In production, use proper key exchange (Diffie-Hellman, RSA) instead of hardcoded keys!

---

## 📁 Project Structure

```
chat-encryption-demo/
│
├── server.py          # Chat server handling multiple clients
│   ├── ChatServer class
│   ├── broadcast() method
│   ├── handle_client() method
│   └── Connection management
│
├── client.py          # Chat client with GUI
│   ├── ChatClient class
│   ├── Tkinter GUI setup
│   ├── Encryption/decryption logic
│   ├── Message send/receive
│   └── Thread for receiving messages
│
└── README.md          # This file
```

---

## 🧪 Testing Scenarios

### Scenario 1: Basic Communication
1. Start server + 2 clients
2. Send messages back and forth
3. Verify both clients see each other's messages

### Scenario 2: Plaintext Wireshark Analysis
1. Disable encryption in both clients
2. Send: "Testing plaintext mode"
3. Capture in Wireshark
4. Verify message is readable in packet bytes

### Scenario 3: Encrypted Wireshark Analysis
1. Enable encryption in both clients
2. Send: "Testing encrypted mode"
3. Capture in Wireshark
4. Verify message is unreadable (base64 gibberish)

### Scenario 4: Mixed Mode (What happens?)
1. Client A: Encryption ON
2. Client B: Encryption OFF
3. A sends encrypted message → B sees: "[Encrypted message - decryption failed]"
4. B sends plaintext → A receives but doesn't decrypt (treats as plaintext)

### Scenario 5: Multiple Clients
1. Start 3+ clients simultaneously
2. Messages broadcast to all connected clients
3. Each client can toggle encryption independently

### Scenario 6: Connection Loss
1. Close one client mid-chat
2. Server logs disconnection
3. Other clients see: "Username left the chat"
4. Wireshark shows FIN/ACK packets (TCP termination)

### Scenario 7: Server Restart
1. Stop server (Ctrl+C)
2. Clients show "Connection lost" error
3. Restart server
4. Clients must reconnect manually (restart client.py)

---

## 🐛 Troubleshooting

### Issue: "Connection refused" when starting client
**Solution:**
- Make sure `server.py` is running first
- Check server shows: `[LISTENING] Server is listening on 127.0.0.1:5555`

### Issue: No packets in Wireshark
**Solution:**
1. Verify you selected **Loopback** interface (not WiFi/Ethernet)
2. Check filter is exactly: `tcp.port == 5555`
3. Send a message to generate traffic
4. Try filter: `tcp.port == 5555 and ip.addr == 127.0.0.1`

### Issue: Wireshark "Permission denied" (Linux)
**Solution:**
```bash
sudo wireshark
# Or properly add yourself to wireshark group:
sudo usermod -aG wireshark $USER
# Then log out and back in
```

### Issue: Can't see message text in Wireshark
**Solution:**
1. Select packet in top pane
2. Expand "Transmission Control Protocol" in middle pane
3. Scroll down to "TCP segment data" and expand
4. Look at **bottom pane, right side (ASCII column)**

### Issue: "Encrypted message - decryption failed"
**Causes:**
- One client has encryption ON, other has it OFF
- Shared secret mismatch (if you modified the code)

**Solution:** Ensure both clients have the same encryption setting

### Issue: Tkinter import error
**Solution:**
```bash
# Ubuntu/Debian
sudo apt-get install python3-tk

# Test if working
python -m tkinter
```

### Issue: Port 5555 already in use
**Solution:**
```bash
# Find and kill the process using port 5555
# Linux/Mac:
lsof -ti:5555 | xargs kill -9

# Windows:
netstat -ano | findstr :5555
taskkill /PID <PID> /F

# Or change port in both server.py and client.py
```

### Issue: Messages not appearing in chat
**Solution:**
- Check server terminal for errors
- Verify both clients are connected (server shows connection count)
- Try restarting both clients

---

## 🔒 Security Notes

### ⚠️ This is an Educational Project

**DO NOT use this for production/real communication!**

### Known Limitations

1. **Hardcoded Key:** The encryption key is the same for all clients and visible in source code
2. **No Key Exchange:** Real apps use Diffie-Hellman or RSA for secure key exchange
3. **No Authentication:** Anyone can connect and impersonate any username
4. **No Message Integrity:** Beyond Fernet's HMAC, there's no message signing
5. **No Perfect Forward Secrecy:** If key is compromised, all past messages are compromised
6. **Localhost Only:** Not designed for internet communication

### Production Best Practices

For real-world secure chat, use:
- **TLS/SSL** for transport layer encryption (HTTPS for web)
- **End-to-end encryption** (Signal Protocol, Matrix)
- **Public key infrastructure** (PKI) for key management
- **Certificate validation** to prevent man-in-the-middle attacks
- **Rate limiting** to prevent spam/DoS
- **User authentication** (passwords, OAuth, etc.)
- **Message signing** with digital signatures
- **Perfect forward secrecy** with ephemeral keys

### Why This Demo Uses Symmetric Encryption

- **Simplicity:** Easy to understand for learning
- **Performance:** Faster than asymmetric encryption
- **Demonstration:** Clearly shows encryption on/off difference in Wireshark

---

## 🎓 Learning Objectives

After completing this project, you'll understand:

✅ **Network fundamentals:**
- TCP/IP protocol basics
- Client-server architecture
- Socket programming in Python

✅ **Security concepts:**
- Why encryption is necessary
- Difference between plaintext and ciphertext
- Symmetric vs asymmetric encryption
- Man-in-the-middle attack vulnerability

✅ **Wireshark skills:**
- Packet capture on localhost
- Applying display filters
- Analyzing TCP streams
- Reading packet payloads
- Identifying encrypted vs plaintext data

✅ **Python programming:**
- Threading for concurrent operations
- Socket programming
- JSON serialization
- GUI development with Tkinter
- Cryptography library usage

✅ **Real-world applications:**
- Why HTTPS matters
- How VPNs protect data
- Why public WiFi is dangerous without encryption
- Importance of end-to-end encryption in messaging apps

---

## 🚀 Extensions & Challenges

Want to take this further? Try implementing:

### Beginner
- [ ] Add timestamps to messages
- [ ] Save chat history to file
- [ ] Add emoji support
- [ ] Change color themes

### Intermediate
- [ ] Implement chat rooms (multiple channels)
- [ ] Add file transfer capability
- [ ] Show "user is typing..." indicator
- [ ] Add message delivery confirmation

### Advanced
- [ ] Implement proper Diffie-Hellman key exchange
- [ ] Add RSA public/private key authentication
- [ ] Implement perfect forward secrecy
- [ ] Add digital signatures for message integrity
- [ ] Build a web-based client with WebSockets

---

## 📚 Additional Resources

### Wireshark
- [Official User Guide](https://www.wireshark.org/docs/wsug_html_chunked/)
- [Display Filter Reference](https://www.wireshark.org/docs/dfref/)
- [Wireshark Tutorial on YouTube](https://www.youtube.com/results?search_query=wireshark+tutorial)

### Python Cryptography
- [Cryptography Library Docs](https://cryptography.io/en/latest/)
- [Real Python: Cryptography Guide](https://realpython.com/python-cryptography/)

### Networking
- [Socket Programming Tutorial](https://docs.python.org/3/howto/sockets.html)
- [TCP/IP Explained](https://en.wikipedia.org/wiki/Internet_protocol_suite)

### Security
- [Applied Cryptography Basics](https://www.coursera.org/learn/crypto)
- [OWASP Security Guidelines](https://owasp.org/)

---

## 📝 License

MIT License - Feel free to use this for educational purposes!

---

## 🤝 Contributing

This is an educational project. Feel free to:
- Fork and modify
- Submit improvements
- Use in teaching/workshops
- Share with students

---

## ⭐ Key Takeaways

1. **Unencrypted data is visible** to anyone on the network path
2. **Encryption makes intercepted data useless** without the key
3. **Always use encryption** for sensitive data (passwords, personal info)
4. **HTTPS is essential** - never enter passwords on HTTP sites
5. **Public WiFi is dangerous** without VPN or encrypted connections
6. **End-to-end encryption matters** - this is why Signal and WhatsApp are secure

---

## 📞 Questions?

If something isn't working:
1. Check the Troubleshooting section above
2. Verify all prerequisites are installed
3. Make sure server is running before clients
4. Check that encryption is on/off in BOTH clients

