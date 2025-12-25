Movie Suggestions — C + HTML/CSS

Files:
- server.c — minimal HTTP server in C (Windows/mingw compatible)
- index.html — frontend page
- styles.css — CSS

Build (Windows with MinGW-w64):

1. Open a terminal in the project folder (c:\Users\laksh\OneDrive\Desktop\MyPrograms).
2. Compile:

   C:\mingw64\bin\gcc.exe -g server.c -o server.exe -lws2_32

3. Run:

   server.exe

4. Open your browser at: http://localhost:8080/

Notes:
- The server serves `index.html` and `styles.css` from the same directory.
- The `/suggest?genre=...` route returns simple hard-coded suggestions.
- No Python is used.
