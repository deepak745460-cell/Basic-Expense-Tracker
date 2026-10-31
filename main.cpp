#include "crow_all.h"
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <map>
#include <chrono>
#include <ctime>

// ─── Data ────────────────────────────────────────────────────────────────────

struct Expense {
    int         id;
    std::string category;
    double      amount;
    std::string date;   // "YYYY-MM-DD"
    std::string note;
};

double              monthlyBudget = 0.0;
std::vector<Expense> expenses;
int                 nextId = 1;

// ─── File Persistence (CSV) ───────────────────────────────────────────────────

const std::string DATA_FILE = "data.csv";

// Returns today's date as "YYYY-MM-DD"
std::string todayDate() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    char buf[11];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", std::localtime(&t));
    return std::string(buf);
}

// Escapes a CSV field — wraps in double-quotes if it contains comma, quote, or newline
std::string csvEscape(const std::string& field) {
    bool needsQuoting = field.find(',')  != std::string::npos
                     || field.find('"')  != std::string::npos
                     || field.find('\n') != std::string::npos;
    if (!needsQuoting) return field;

    std::string escaped = "\"";
    for (char c : field) {
        if (c == '"') escaped += "\"\"";  // escape inner double-quotes
        else          escaped += c;
    }
    escaped += "\"";
    return escaped;
}

// Parses one CSV line into fields, handles quoted fields correctly
std::vector<std::string> parseCsvLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    bool inQuotes = false;

    for (int i = 0; i < (int)line.size(); i++) {
        char c = line[i];
        if (inQuotes) {
            if (c == '"' && i + 1 < (int)line.size() && line[i+1] == '"') {
                field += '"';   // "" inside quotes → single "
                i++;
            } else if (c == '"') {
                inQuotes = false;
            } else {
                field += c;
            }
        } else {
            if      (c == '"')  inQuotes = true;
            else if (c == ',')  { fields.push_back(field); field.clear(); }
            else                field += c;
        }
    }
    fields.push_back(field);  // push last field
    return fields;
}

// Saves all data to data.csv
void saveData() {
    std::ofstream file(DATA_FILE);

    // Line 1: metadata (budget and next ID)
    file << "BUDGET," << monthlyBudget << ",NEXTID," << nextId << "\n";

    // Line 2: column headers
    file << "id,category,amount,date,note\n";

    // Lines 3+: one row per expense
    for (const auto& e : expenses) {
        file << e.id                    << ","
             << csvEscape(e.category)   << ","
             << e.amount                << ","
             << csvEscape(e.date)       << ","
             << csvEscape(e.note)       << "\n";
    }
}

// Loads saved data from data.csv on startup
void loadData() {
    std::ifstream file(DATA_FILE);
    if (!file.is_open()) return;

    std::string line;

    // Line 1: metadata — format: BUDGET,<val>,NEXTID,<val>
    if (!std::getline(file, line)) return;
    auto meta = parseCsvLine(line);
    if (meta.size() >= 4) {
        monthlyBudget = std::stod(meta[1]);
        nextId        = std::stoi(meta[3]);
    }

    // Line 2: header — skip
    std::getline(file, line);

    // Lines 3+: expense rows
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        auto f = parseCsvLine(line);
        if (f.size() < 5) continue;  // skip incomplete rows

        expenses.push_back({
            std::stoi(f[0]),   // id
            f[1],              // category
            std::stod(f[2]),   // amount
            f[3],              // date
            f[4]               // note
        });
    }
}

// ─── Helpers ─────────────────────────────────────────────────────────────────

crow::response ok(std::string message = "") {
    crow::json::wvalue res;
    res["status"] = "success";
    if (!message.empty()) res["message"] = message;
    return crow::response(res);
}

crow::response err(int code, std::string message) {
    crow::json::wvalue res;
    res["status"]  = "error";
    res["message"] = message;
    return crow::response(code, res);
}

// ─── Main ────────────────────────────────────────────────────────────────────

int main() {
    loadData(); // Load saved data on startup

    crow::SimpleApp app;
    crow::mustache::set_base("templates");

    // GET / → HTML page
    CROW_ROUTE(app, "/")
    ([]() {
        return crow::mustache::load("index.html").render();
    });

    // ── Budget ───────────────────────────────────────────────────────────────

    // POST /set-budget → {"budget": 5000}
    CROW_ROUTE(app, "/set-budget").methods(crow::HTTPMethod::Post)
    ([](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body || !body.has("budget"))
            return err(400, "budget field missing");

        monthlyBudget = body["budget"].d();
        saveData();
        return ok("Budget updated successfully");
    });

    // ── Expenses ─────────────────────────────────────────────────────────────

    // POST /add-expense → {"category":"Food", "amount":200, "date":"2024-01-15", "note":"Lunch"}
    CROW_ROUTE(app, "/add-expense").methods(crow::HTTPMethod::Post)
    ([](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body || !body.has("amount") || !body.has("category"))
            return err(400, "amount or category field missing");

        std::string date = body.has("date") ? std::string(body["date"].s()) : todayDate();
        std::string note = body.has("note") ? std::string(body["note"].s()) : std::string("");

        expenses.push_back({ nextId++, body["category"].s(), body["amount"].d(), date, note });
        saveData();
        return ok();
    });

    // DELETE /delete-expense → {"id": 3}
    CROW_ROUTE(app, "/delete-expense").methods(crow::HTTPMethod::Delete)
    ([](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body || !body.has("id"))
            return err(400, "id field missing");

        int targetId = body["id"].i();
        auto it = std::remove_if(expenses.begin(), expenses.end(),
            [targetId](const Expense& e) { return e.id == targetId; });

        if (it == expenses.end())
            return err(404, "Expense not found");

        expenses.erase(it, expenses.end());
        saveData();
        return ok("Expense deleted");
    });

    // ── Dashboard + Filters ──────────────────────────────────────────────────

    // GET /dashboard?category=Food&search=lunch
    // Optional query params: category (filter), search (keyword in note/category)
    CROW_ROUTE(app, "/dashboard").methods(crow::HTTPMethod::Get)
    ([](const crow::request& req) {
        std::string filterCategory = req.url_params.get("category") ?
                                     req.url_params.get("category") : "";
        std::string searchQuery    = req.url_params.get("search")   ?
                                     req.url_params.get("search")   : "";

        // lowercase helper
        auto toLower = [](std::string s) {
            std::transform(s.begin(), s.end(), s.begin(), ::tolower);
            return s;
        };

        std::string lowerSearch = toLower(searchQuery);

        // Apply filter and search
        std::vector<Expense> filtered;
        for (const auto& e : expenses) {
            bool catMatch  = filterCategory.empty() || toLower(e.category) == toLower(filterCategory);
            bool srcMatch  = lowerSearch.empty()
                             || toLower(e.category).find(lowerSearch) != std::string::npos
                             || toLower(e.note).find(lowerSearch)     != std::string::npos;
            if (catMatch && srcMatch)
                filtered.push_back(e);
        }

        // Calculate total spending (filtered)
        double totalSpent = 0.0;
        for (const auto& e : filtered) totalSpent += e.amount;

        // ⚠️ Budget Alert — warn when spending reaches 80% of budget
        double allTotal = 0.0;
        for (const auto& e : expenses) allTotal += e.amount;
        bool   budgetAlert    = (monthlyBudget > 0) && (allTotal >= monthlyBudget * 0.8);
        double budgetUsedPct  = (monthlyBudget > 0) ? (allTotal / monthlyBudget * 100.0) : 0.0;

        // 📊 Build category-wise summary
        std::map<std::string, double> catSummary;
        for (const auto& e : expenses) catSummary[e.category] += e.amount;

        // Build response JSON
        crow::json::wvalue res;
        res["budget"]          = monthlyBudget;
        res["total_spent"]     = totalSpent;
        res["budget_used_pct"] = budgetUsedPct;
        res["budget_alert"]    = budgetAlert;

        // Filtered expenses array
        for (int i = 0; i < (int)filtered.size(); i++) {
            res["expenses"][i]["id"]       = filtered[i].id;
            res["expenses"][i]["category"] = filtered[i].category;
            res["expenses"][i]["amount"]   = filtered[i].amount;
            res["expenses"][i]["date"]     = filtered[i].date;
            res["expenses"][i]["note"]     = filtered[i].note;
        }

        // Category summary array
        int ci = 0;
        for (const auto& [cat, total] : catSummary) {
            res["category_summary"][ci]["category"] = cat;
            res["category_summary"][ci]["total"]    = total;
            ci++;
        }

        return crow::response(res);
    });

    // GET /categories → unique categories list
    CROW_ROUTE(app, "/categories").methods(crow::HTTPMethod::Get)
    ([]() {
        std::vector<std::string> cats;
        for (const auto& e : expenses) {
            if (std::find(cats.begin(), cats.end(), e.category) == cats.end())
                cats.push_back(e.category);
        }
        crow::json::wvalue res;
        for (int i = 0; i < (int)cats.size(); i++)
            res["categories"][i] = cats[i];
        return crow::response(res);
    });

    app.port(8080).multithreaded().run();
}
