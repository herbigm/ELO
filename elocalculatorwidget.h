#ifndef ELOCALCULATORWIDGET_H
#define ELOCALCULATORWIDGET_H

/*
 *
 * Code for the calculations mostly used from https://github.com/0xmanjoos/libexpr
 *
 */

#include <vector>

#include <QWidget>
#include <QBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QLineEdit>

enum ExpressionToken {
    EXP = '^',
    MUL = '*',
    DIV = '/',
    MOD = '%',
    ADD = '+',
    SUB = '-',
    NEG = '_', // custom negation operator
    LPAR = '(',
    RPAR = ')',
    LITERAL = 1,
    IDENTIFIER = 2
};

struct Token {
    Token(char, char, double, bool);
    ExpressionToken tok;
    char precedence;
    bool op;
    double value;
};
typedef struct Token token_t;


class ELOcalculatorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ELOcalculatorWidget(QWidget *parent = nullptr);

private:
    std::vector<token_t*>* tokExpr(char* buffer);
    std::vector<token_t*>* infix2rpn(std::vector<token_t*>*);
    double evaluateOp(double, double, char);
    double executeExpr(std::vector<token_t*>*);
    void doCalculation();
    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);
    void on_edit_textEdited(const QString &arg1);

    // widgets
    QVBoxLayout *layout1;
    QHBoxLayout *layout2;
    QListWidget *listWidget;
    QLabel *label;
    QPushButton *btn;
    QLineEdit *edit;

};

#endif // ELOCALCULATORWIDGET_H
