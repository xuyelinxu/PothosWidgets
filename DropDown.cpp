// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Pothos/Object/Containers.hpp>
#include <QComboBox>
#include <QLabel>
#include <QHBoxLayout>
#include <vector>

/***********************************************************************
 * |PothosDoc Drop Down
 *
 * The drop down widget allows for selection of a value
 * from a drop-down/combo-box menu.
 *
 * |category /Widgets
 * |keywords dropdown combobox
 *
 * |param title The name of the value displayed by this widget
 * |default "My Menu Value"
 *
 * |param value The initial selection for this radio group.
 * |default 42
 *
 * |param options A list of options to choose from.
 * Options take the form of an Object vector
 * where each entry is another Object vector
 * containing a displayable name, and a value for the option.
 *
 * |default [["Opt0", 42], ["Opt1", "xyz"]]
 *
 * |mode graphWidget
 * |factory /widgets/drop_down()
 * |setter setTitle(title)
 * |setter setOptions(options)
 * |setter setValue(value)
 **********************************************************************/
class DropDown : public QWidget, public Pothos::Block
{
    Q_OBJECT
public:

    static Block *make(void)
    {
        return new DropDown();
    }

    DropDown(void):
        _label(new QLabel(this)),
        _comboBox(new QComboBox(this))
    {
        auto hlayout = new QHBoxLayout(this);
        hlayout->setContentsMargins(QMargins());
        hlayout->setSpacing(1);
        hlayout->addWidget(_label);
        hlayout->addWidget(_comboBox);

        this->registerCall(this, POTHOS_FCN_TUPLE(DropDown, widget));
        this->registerCall(this, POTHOS_FCN_TUPLE(DropDown, setTitle));
        this->registerCall(this, POTHOS_FCN_TUPLE(DropDown, setValue));
        this->registerCall(this, POTHOS_FCN_TUPLE(DropDown, setOptions));
        this->registerSignal("valueChanged");

        qRegisterMetaType<Pothos::Object>("Pothos::Object");
        qRegisterMetaType<Pothos::ObjectVector>("Pothos::ObjectVector");
        connect(_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(handleIndexChanged(int)));
    }

    QWidget *widget(void)
    {
        return this;
    }

    void activate(void)
    {
        //emit current value when design becomes active
        this->emitSignalArgs("valueChanged", Pothos::ObjectVector(1, this->value()));
    }

    Pothos::Object value(void) const
    {
        const auto index = _comboBox->currentIndex();
        if (index < 0) return Pothos::Object();
        if (size_t(index) >= _optionValues.size()) return Pothos::Object();
        return _optionValues.at(index);
    }

    void setValue(const Pothos::Object &value)
    {
        QMetaObject::invokeMethod(this, "__setValue", Qt::QueuedConnection, Q_ARG(Pothos::Object, value));
    }

    void setOptions(const Pothos::ObjectVector &options)
    {
        QMetaObject::invokeMethod(this, "__setOptions", Qt::QueuedConnection, Q_ARG(Pothos::ObjectVector, options));
    }

    void setTitle(const QString &title)
    {
        const auto text = QString("<b>%1</b>").arg(title.toHtmlEscaped());
        //cannot call setText in calling thread, forward to the label slot
        QMetaObject::invokeMethod(_label, "setText", Qt::QueuedConnection, Q_ARG(QString, text));
    }

private slots:

    void __setOptions(const Pothos::ObjectVector &options)
    {
        auto oldValue = this->value();
        _comboBox->clear();
        _optionValues.clear();

        for (const auto &option : options)
        {
            if (not option.canConvert(typeid(Pothos::ObjectVector))) throw Pothos::DataFormatException("DropDown::setOptions()", "entry is not ObjectVector");
            auto optPair = option.convert<Pothos::ObjectVector>();
            if (optPair.size() != 2) throw Pothos::DataFormatException("DropDown::setOptions()", "entry must be ObjectVector of size == 2");
            if (not optPair.at(0).canConvert(typeid(QString))) throw Pothos::DataFormatException("DropDown::setOptions()", "entry[0] must be a string");
            auto title = optPair.at(0).convert<QString>();
            auto value = optPair.at(1);
            _comboBox->addItem(title);
            _optionValues.push_back(value);
        }

        this->setValue(oldValue);
    }

    void __setValue(const Pothos::Object &value)
    {
        for (int i = 0; i < _comboBox->count(); i++)
        {
            if (_optionValues.at(i).equals(value)) _comboBox->setCurrentIndex(i);
        }
    }

    void handleIndexChanged(int)
    {
        this->emitSignalArgs("valueChanged", Pothos::ObjectVector(1, this->value()));
    }

private:
    QLabel *_label;
    QComboBox *_comboBox;
    std::vector<Pothos::Object> _optionValues;
};

static Pothos::BlockRegistry registerDropDown(
    "/widgets/drop_down", &DropDown::make);

#include "DropDown.moc"