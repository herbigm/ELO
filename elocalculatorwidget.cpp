#include "elocalculatorwidget.h"

#include <stack>
#include <queue>
#include <cmath>
#include <QDebug>
#include <QLocale>

#ifdef  _WIN32
    #include <locale.h>
#endif


Token::Token(char token, char prec, double val, bool operation){
    tok = (ExpressionToken)token;
    precedence = prec;
    value = val;
    op = operation;
    return;
}

ELOcalculatorWidget::ELOcalculatorWidget(QWidget *parent) : QWidget(parent)
{
    layout1 = new QVBoxLayout(this);
    layout2 = new QHBoxLayout();
    listWidget = new QListWidget(this);
    label = new QLabel(tr("expression: "), this);
    btn = new QPushButton(tr("calculate"), this);
    edit = new QLineEdit(this);

    layout1->addWidget(listWidget);
    layout2->addWidget(label);
    layout2->addWidget(edit);
    layout2->addWidget(btn);
    layout1->addLayout(layout2);

    connect(btn, &QPushButton::pressed, this, &ELOcalculatorWidget::doCalculation);
    connect(edit, &QLineEdit::returnPressed, this, &ELOcalculatorWidget::doCalculation);
    connect(listWidget, &QListWidget::itemDoubleClicked, this, &ELOcalculatorWidget::on_listWidget_itemDoubleClicked);
    connect(edit, &QLineEdit::textEdited, this, &ELOcalculatorWidget::on_edit_textEdited);
}

std::vector<token_t *> *ELOcalculatorWidget::tokExpr(char *buffer)
{
    /*
     * Creates a vector of tokens from a string which can than be used in the Shunting-yard algorithm.
     */

    std::vector<token_t*>* stream = new std::vector<token_t*>;
    char* cursor = buffer, prec = 0;
    bool op = false;

    while (*cursor) {
        switch (*cursor) {
            case ' ': cursor++; continue;
#ifdef  _WIN32
            case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
#else
            case '0' ... '9':
#endif
                stream->push_back(new token_t((char)LITERAL, (char)0, strtod(cursor, &cursor), op));
                continue;
            case '(': [[fallthrough]];
            case ')':
                stream->push_back(new token_t(*cursor, (char)0, (double)0, op));
                 cursor++;
                continue;
            case '^': prec = 5;op = true;goto push_tok;
            case '*': prec = 3;op = true;goto push_tok;
            case '/': prec = 3;op = true;goto push_tok;
            case '%': prec = 3;op = true;goto push_tok;
            case '+': prec = 2;op = true;goto push_tok;
            case '-': prec = 2;op = true;goto push_tok;
            default: listWidget->addItem(tr("Unknown expression ") + *cursor); delete stream; return nullptr;
        }
        cursor++;
    push_tok:
        stream->push_back(new token_t(*cursor, (char)prec, (double)0, op));
        cursor++; prec = 0; op = false;
        continue;
    }
    if (stream->empty()) {
        listWidget->addItem(tr("Nothing to calculate."));
        stream = nullptr;
    }
    return stream;
}

std::vector<token_t*>* ELOcalculatorWidget::infix2rpn(std::vector<token_t*> *TokenStream) {
    /*
     * Converts infix noation to reversed polish notation.
     */

    /* error handling */
    if (TokenStream == nullptr || TokenStream->empty())
        return nullptr;

    /* output queue */
    std::vector<token_t*>* Output = new std::vector<token_t*>;

    /* operators stack */
    std::stack<token_t*> Operators;

    char o1 = 0, o2 = 0;
    int i, n = TokenStream->size();

    /* temporary current & previous token */
    token_t* temp, *prev;
    char temptok = 0;

    for (i = 0; i < n; i++) {
        temp = (*TokenStream)[i];
        temptok = temp->tok;
        if (temptok == '-') {
            if (i) {
                prev = (*TokenStream)[i-1];
                if (prev->op || prev->tok == LPAR) {
                    temp->tok = NEG;
                    temp->precedence = 4;
                }
            } else {
                temp->tok = NEG;
                temp->precedence = 4;
            }
        }

        if (temptok == LITERAL) {
            Output->push_back(temp);
            continue;
        }

        if (temp->op) {

            if (Operators.empty()) {
                Operators.push(temp);
                continue;
            }

            o1 = temp->precedence,
            o2 = Operators.top()->precedence;

            while ((!Operators.empty() && Operators.top()->op)
                && (o2 > o1 || (o2 == o1 && temp->tok < '^'))) {
                Output->push_back(Operators.top());
                Operators.pop();
                if (!Operators.empty()) {
                    o2 = Operators.top()->precedence;
                }
            }
            Operators.push(temp);
            continue;
        }

        if (temptok == LPAR) {
            Operators.push(temp);
            continue;
        }
        if (temptok == RPAR) {
            if (Operators.empty()) {
                listWidget->addItem(tr("Parenthesis missmatch: to many closing parenthesis."));
                delete Output;
                return nullptr;
            }
            while (!Operators.empty() && Operators.top()->tok != LPAR) {
                Output->push_back(Operators.top());
                Operators.pop();
            }
        }

        if (Operators.empty()) {
            listWidget->addItem(tr("Parenthesis missmatch: to many closing parenthesis."));
            delete Output;
            return nullptr;
        }
        free((*TokenStream)[i]);
        free((void*)Operators.top());
        Operators.pop();
    }

    while (!Operators.empty()) {
        if (Operators.top()->tok == LPAR) {
            listWidget->addItem(tr("Parenthesis missmatch: to many opening parenthesis."));
            delete Output;
            return nullptr;
        }
        Output->push_back(Operators.top());
        Operators.pop();
    }

    delete TokenStream;
    return Output;
}

double ELOcalculatorWidget::evaluateOp(double x, double y, char op) {
    /*
     * Simply evaluates the operation with two doubles.
     */
    switch (op) {
        case '^': return pow(x, y);
        case '_': return (double)(*(long int*)&y ^ 0x80000000);
        case '*': return x * y;
        case '/': return x / y;
        case '%': return fmod(x, y);
        case '+': return x + y;
        case '-': return x - y;
    }
    return -1;
}

double ELOcalculatorWidget::executeExpr(std::vector<token_t*>* in) {
    /*
     * Evaluates the expression using the reversed polish notation.
     */

    /* error handling */
    if (in == nullptr)
        return 0;

    std::stack<double> OutputStack;
    /*
        we can have the stack as a double type as we never push a token onto the stack. it will
        only hold literal values; which then makes no sense to push the entire token. it just
        increases memory usage and we have to constantly call methods. bleh
    */
    double o1 = 0, o2 = 0;
    for (auto& it: *in) {
        if (it->tok == LITERAL) {
            OutputStack.push(it->value);
            free(it);
            continue;
        }

        if (it->op) {
            if (OutputStack.empty()) {
                listWidget->addItem(tr("Error in equation. "));
                return 0;
            }
            if (it->tok == NEG) {
                o1 = OutputStack.top();
                OutputStack.pop();
                OutputStack.push(o1 * -1);
                free(it);
                continue;
            }

            /*
              seems very inefficient, why does pop() not return the value??
              we also have to do some checking to make sure stack is not empty, assertions probably
            */

            o1 = OutputStack.top();
            OutputStack.pop();
            if (OutputStack.empty()) {
                listWidget->addItem(tr("Error in equation. "));
                return 0;
            }
            o2 = OutputStack.top();
            OutputStack.pop();
            OutputStack.push(evaluateOp(o2, o1, it->tok));
            free(it);
            continue;
        }
    }
    if (OutputStack.size() != 1) {
        listWidget->addItem(tr("Error in equation. "));
        return 0;
    }
    delete in;
    return OutputStack.top();
}

void ELOcalculatorWidget::doCalculation()
{
    QString eqn = edit->text();
    if (!eqn.isEmpty()) {
        listWidget->addItem(eqn);
        struct lconv* lc = localeconv();
        eqn = eqn.replace(",", lc->decimal_point);
        eqn = eqn.replace(".", lc->decimal_point);
        listWidget->addItem("\t= " + QString::number(executeExpr(infix2rpn(tokExpr(eqn.toUtf8().data())))).replace(".", lc->decimal_point));
        listWidget->scrollToItem(listWidget->item(listWidget->count()-1));
        edit->clear();
        edit->setFocus();
    }
}

void ELOcalculatorWidget::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    QString text = item->text().trimmed();
    if (text.startsWith("="))
        text = text.mid(2);
    edit->setText(edit->text() + text);
}

void ELOcalculatorWidget::on_edit_textEdited(const QString &arg1)
{
    if (arg1 == "+" || arg1 == "-" || arg1 == "*" || arg1 == "/") {
        QString text = listWidget->item(listWidget->count()-1)->text().trimmed().mid(2);
        edit->setText(text + edit->text());
    }
}
