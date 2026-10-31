# 💰 Basic Expense Tracker

A simple personal expense tracker with a **C++ backend** (Crow framework) and an HTML/CSS/JS frontend.

---

## 📁 Project Structure

```
Basic Expense Tracker/
├── main.cpp              ← C++ backend source code
├── main.exe              ← Compiled executable (run this)
├── crow_all.h            ← Crow web framework (header-only)
├── data.csv              ← Auto-generated data file (expenses saved here)
├── compile.bat           ← Double-click to recompile
├── run.bat               ← Double-click to start the server
└── templates/
    └── index.html        ← Frontend UI (edit this for UI changes)
```

---

## ▶️ How to Run

### Option 1 — Double-click `run.bat`

### Option 2 — CMD
```cmd
cd "c:\Users\Deepak\Downloads\Basic Expense Tracker"
main.exe
```

Then open your browser and go to:
```
http://localhost:8080
```

Press `Ctrl+C` in the terminal to stop the server.

---

## 🔨 How to Compile (Build `main.exe` from source)

### Option 1 — Double-click `compile.bat` ✅ (Easiest)

### Option 2 — CMD
```cmd
cd "c:\Users\Deepak\Downloads\Basic Expense Tracker"

g++ main.cpp -o main.exe -I"C:\msys64\ucrt64\include" -lws2_32 -lpthread -lmswsock -static-libgcc -static-libstdc++ -static
```

> ⚠️ **If you get `g++ not found` error**, run this first:
> ```cmd
> set PATH=C:\msys64\ucrt64\bin;C:\msys64\usr\bin;%PATH%
> ```
> Then run the compile command again.

Compilation takes **~15–20 seconds** — this is normal (Crow is a large framework).

---

## 🔁 When to Recompile?

| What you changed | Action needed |
|-----------------|---------------|
| `main.cpp` | ✅ Recompile → run `compile.bat` |
| `templates/index.html` | ❌ No recompile — just refresh browser (`Ctrl+Shift+R`) |

---

## 🌐 API Endpoints

| Method | Endpoint | Body | Description |
|--------|----------|------|-------------|
| `GET` | `/` | — | Serves the HTML page |
| `POST` | `/set-budget` | `{"budget": 5000}` | Set monthly budget |
| `POST` | `/add-expense` | `{"category":"Food","amount":200,"date":"2024-10-31","note":"Lunch"}` | Add a new expense |
| `DELETE` | `/delete-expense` | `{"id": 3}` | Delete an expense by ID |
| `GET` | `/dashboard` | — | Get budget, expenses, and category summary |
| `GET` | `/dashboard?category=Food` | — | Filter expenses by category |
| `GET` | `/dashboard?search=lunch` | — | Search expenses by keyword |
| `GET` | `/categories` | — | Get all unique categories |

---

## ✨ Features

- ✅ Set monthly budget
- ✅ Add expenses with category, amount, date, and note
- ✅ Delete any expense
- ✅ Filter by category
- ✅ Search by keyword (category or note)
- ✅ Category-wise spending summary (bar chart)
- ✅ Budget progress bar (green → yellow → red)
- ✅ Budget alert when 80%+ is used
- ✅ Data saved to `data.csv` (persists after server restart)

---

## 🛠️ Requirements

- **g++** via [MSYS2 UCRT64](https://www.msys2.org/) — install from `C:\msys64\ucrt64\bin`
- **ASIO library** — install via MSYS2:
  ```
  pacman -S mingw-w64-ucrt-x86_64-asio
  ```
