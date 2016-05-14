#ifndef SIP_MSG_BUILDER_H
#define SIP_MSG_BUILDER_H

#include <QDomDocument>
#include <QDomElement>
#include <QVariant>

// �������Ϣ������
//<?xml version=1.0 encoding=utf-8?>
//<coolview command="command����" type="type����">
//  <name����>value����</name����>
//   ...
//</coolview>
// �ڷ����ַ���ʱ��ѡ��-1������ȥ���˻��к�element֮��Ŀո�

class SipMsgBuilder {
public:
  SipMsgBuilder(const QString &command, const QString &type = QString());
  ~SipMsgBuilder();

  bool AddParameter(const QString &name, const QVariant &value);

  template <typename T>
  void AddArgument(const QString &name, const T &value) {
    AddArgument(command_element_, name, value);
  }

  QString GetMsg() const {
    return doc_.toString(-1);
  }

private:
  template <typename T>
  void AddArgument(QDomElement &parent, const QString &name, const T &value) 
  {
    QDomElement argument_element = doc_.createElement(name);
    argument_element.appendChild(doc_.createTextNode(QString::number(value)));
    parent.appendChild(argument_element);
  }

  void AddArgument(QDomElement &parent, const QString &name, const QString &value);
  void AddArgument(QDomElement &parent, const QString &name, bool value);

  QDomDocument doc_;
  QDomElement command_element_;
};


#endif // SIP_MSG_BUILDER_H
