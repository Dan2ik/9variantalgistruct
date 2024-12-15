#include <iostream>
#include <stack>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <sstream>
#include <cctype>
#include <windows.h>
using namespace std;

// Структура для узла дерева
struct TreeNode {
    char value; // Значение узла (оператор или операнд)
    TreeNode* left;  // Левый дочерний узел
    TreeNode* right; // Правый дочерний узел
    TreeNode(char val) : value(val), left(nullptr), right(nullptr) {}
};

// Удаление дерева для предотвращения утечек памяти
void deleteTree(TreeNode* root) {
    if (!root) return;
    deleteTree(root->left);
    deleteTree(root->right);
    delete root;
}

// Проверка правильности ввода арифметического выражения
bool isValidExpression(const string& expression) {
    stack<char> stack;
    for (char ch : expression) {
        if (ch == '(') {
            stack.push(ch); // Открывающая скобка
        }
        else if (ch == ')') {
            if (stack.empty() || stack.top() != '(') {
                return false; // Найдена лишняя закрывающая скобка
            }
            stack.pop(); // Закрывающая скобка успешно сопоставлена
        }
        else if (!isalnum(ch) && ch != '+' && ch != '-' && ch != '*' && ch != '/' && ch != ' ') {
            return false; // Найден недопустимый символ
        }
    }
    return stack.empty(); // Все скобки должны быть закрыты
}

// Функция для определения приоритета операторов
int getPriority(char op) {
    if (op == '+' || op == '-') return 1; // Низкий приоритет
    if (op == '*' || op == '/') return 2; // Высокий приоритет
    return 0; // Не оператор
}

// Преобразование инфиксного выражения в постфиксную нотацию (обратная польская запись)
string toPostfix(const string& expression) {
    stack<char> stack; // Стек для операторов
    string postfix;    // Результат в постфиксной нотации
    for (char ch : expression) {
        if (isspace(ch)) continue; // Игнорирование пробелов
        if (isalnum(ch)) {
            postfix += ch; // Операнд добавляется к результату
        }
        else if (ch == '(') {
            stack.push(ch); // Открывающая скобка
        }
        else if (ch == ')') {
            // Перемещаем операторы в результат до открывающей скобки
            while (!stack.empty() && stack.top() != '(') {
                postfix += stack.top();
                stack.pop();
            }
            if (!stack.empty()) stack.pop(); // Убираем открывающую скобку
        }
        else {
            // Перемещаем операторы с более высоким или равным приоритетом
            while (!stack.empty() && getPriority(stack.top()) >= getPriority(ch)) {
                postfix += stack.top();
                stack.pop();
            }
            stack.push(ch); // Текущий оператор в стек
        }
    }
    // Перемещаем оставшиеся операторы в результат
    while (!stack.empty()) {
        postfix += stack.top();
        stack.pop();
    }
    return postfix;
}

// Построение дерева из постфиксной нотации
TreeNode* buildTree(const string& postfix) {
    stack<TreeNode*> stack; // Стек для построения дерева
    for (char ch : postfix) {
        if (isalnum(ch)) {
            stack.push(new TreeNode(ch)); // Операнд становится листом дерева
        }
        else {
            // Проверка: должно быть минимум два узла для оператора
            if (stack.size() < 2) throw invalid_argument("Некорректное постфиксное выражение");
            TreeNode* right = stack.top(); stack.pop(); // Правый операнд
            TreeNode* left = stack.top(); stack.pop();  // Левый операнд
            TreeNode* node = new TreeNode(ch); // Создаем узел для оператора
            node->left = left;
            node->right = right;
            stack.push(node); // Добавляем узел в стек
        }
    }
    if (stack.size() != 1) throw invalid_argument("Некорректное постфиксное выражение");
    return stack.top(); // Корень дерева
}

// Вычисление значения выражения по дереву
int evaluateTree(TreeNode* root, const unordered_map<char, int>& variables) {
    if (!root) return 0;
    if (isalnum(root->value)) {
        if (isalpha(root->value)) {
            // Проверяем, есть ли значение переменной
            if (variables.find(root->value) == variables.end()) {
                throw invalid_argument("Неопределенная переменная: " + string(1, root->value));
            }
            return variables.at(root->value); // Значение переменной
        }
        return root->value - '0'; // Преобразование символа числа в целое число
    }
    // Рекурсивный расчет левого и правого поддеревьев
    int leftVal = evaluateTree(root->left, variables);
    int rightVal = evaluateTree(root->right, variables);
    switch (root->value) {
    case '+': return leftVal + rightVal;
    case '-': return leftVal - rightVal;
    case '*': return leftVal * rightVal;
    case '/':
        if (rightVal == 0) throw invalid_argument("Деление на ноль");
        return leftVal / rightVal;
    default: throw invalid_argument("Недопустимый оператор");
    }
}

int main() {
    setlocale(LC_ALL, "rus");
    
    string expression;
    cout << "Введите арифметическое выражение: ";
    getline(cin, expression);

    if (!isValidExpression(expression)) {
        cerr << "Некорректное выражение" << endl;
        return 1;
    }

    try {
        string postfix = toPostfix(expression); // Преобразование в постфиксную нотацию
        TreeNode* root = buildTree(postfix); // Построение дерева

        // Считывание значений переменных
        unordered_map<char, int> variables;
        cout << "Введите значения переменных (например, a=5 b=3), или нажмите Enter для завершения: " << endl;
        string input;
        while (true) {
            getline(cin, input);
            if (input.empty()) break; // Завершение ввода
            istringstream iss(input);
            char var;
            int val;
            if (iss >> var && iss.get() == '=' && iss >> val) {
                variables[var] = val; // Сохранение переменной
            }
            else {
                cerr << "Некорректный формат ввода" << endl;
            }
        }

        int result = evaluateTree(root, variables); // Вычисление результата
        cout << "Результат: " << result << endl;

        deleteTree(root); // Удаление дерева (освобождение памяти)
    }
    catch (const exception& e) {
        cerr << "Ошибка: " << e.what() << endl;
    }

    return 0;
}
