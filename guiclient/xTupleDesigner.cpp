/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "xTupleDesigner.h"

#include <QDomDocument>
#include <QFile>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QTextStream>
#include <QtDesigner/QDesignerComponents>
#include <QtDesigner/QDesignerIntegrationInterface>
#include <QtDesigner>

// TODO: can we live without this?
// copied from .../qt-mac-commercial-src-4.4.3/tools/designer/src/lib/shared/qdesigner_integration_p.h
#include "qdesigner_integration_p.h"

// TODO: (re)move the following when the UI gets sorted out
#include <QHBoxLayout>
#include <QPushButton>
#include <QSplitter>

#include "xTupleDesignerActions.h"
#include "xmainwindow.h"

class WidgetBoxWindow : public XMainWindow
{
  public:
    WidgetBoxWindow(xTupleDesigner *);

  private:
    QDesignerWidgetBoxInterface *_widgetbox;
};

WidgetBoxWindow::WidgetBoxWindow(xTupleDesigner *parent)
  : XMainWindow(parent, "_widgetBoxWindow", Qt::Tool)
{
  _widgetbox = QDesignerComponents::createWidgetBox(parent->formeditor(), this);
  parent->formeditor()->setWidgetBox(_widgetbox);
  setCentralWidget(_widgetbox);
  setWindowTitle(tr("Widget Box"));
}

class ObjectInspectorWindow : public XMainWindow
{
  public:
    ObjectInspectorWindow(xTupleDesigner *);

  private:
    QDesignerObjectInspectorInterface *_objinsp;
};

ObjectInspectorWindow::ObjectInspectorWindow(xTupleDesigner *parent)
  :XMainWindow(parent, "_objectInspectorWindow", Qt::Tool)
{
  _objinsp = QDesignerComponents::createObjectInspector(parent->formeditor(),
                                                        this);
  parent->formeditor()->setObjectInspector(_objinsp);
  setCentralWidget(_objinsp);
  setWindowTitle(tr("Object Inspector"));
}

class PropertyEditorWindow : public XMainWindow
{
  public:
    PropertyEditorWindow(xTupleDesigner *);

  private:
    QDesignerPropertyEditorInterface *_propeditor;
};

PropertyEditorWindow::PropertyEditorWindow(xTupleDesigner *parent)
  : XMainWindow(parent, "_propertyEditorWindow", Qt::Tool)
{
  _propeditor = QDesignerComponents::createPropertyEditor(parent->formeditor(),
                                                          this);
  parent->formeditor()->setPropertyEditor(_propeditor);
  setCentralWidget(_propeditor);
  setWindowTitle(tr("Property Editor"));
}

class SignalSlotEditorWindow : public XMainWindow
{
  public:
    SignalSlotEditorWindow(xTupleDesigner *);

  private:
    QWidget *_sloteditor;
};

SignalSlotEditorWindow::SignalSlotEditorWindow(xTupleDesigner *parent)
  : XMainWindow(parent, "_signalSlotEditorWindow", Qt::Tool)
{
  _sloteditor = QDesignerComponents::createSignalSlotEditor(parent->formeditor(),
                                                          this);
  setCentralWidget(_sloteditor);
  setWindowTitle(tr("Signal/Slot Editor"));
}

xTupleDesigner::xTupleDesigner(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  _formEnabled = true;
  _formId      = -1;
  _notes       = QString();
  _order       = 0;
  _source      = 0;

  _formeditor = QDesignerComponents::createFormEditor(this);
  if (! _formeditor)
  {
    QMessageBox::information(this, QString("Cannot edit the UI"), 
                             QString("<p>The application could not open the "
                                     "form editor."));
    return;
  }

  QDesignerComponents::createTaskMenu(_formeditor, this);

  QList <QObject*>builtinPlugins = QPluginLoader::staticInstances();
  for (int i = 0; i < builtinPlugins.size(); i++)
  {
    QDesignerFormEditorPluginInterface *plugin = qobject_cast<QDesignerFormEditorPluginInterface*>(builtinPlugins.at(i));
    if (plugin && ! plugin->isInitialized())
      plugin->initialize(_formeditor);
  }
  QDesignerComponents::initializePlugins(_formeditor);

  _actions = new xTupleDesignerActions(this);

  _menubar = new QMenuBar(this);
  _menubar->setObjectName("_xTupleDesignerMenuBar");

  _filemenu = _menubar->addMenu(tr("&File"));
  foreach (QAction *a, _actions->fileActions()->actions())
    _filemenu->addAction(a);

  _editmenu = _menubar->addMenu(tr("&Edit"));
  foreach (QAction *a, _actions->editActions()->actions())
    _editmenu->addAction(a);

  _formmenu = _menubar->addMenu(tr("&Form"));
  foreach (QAction *a, _actions->formActions()->actions())
    _formmenu->addAction(a);

  _toolmenu = _menubar->addMenu(tr("&Tool"));
  foreach (QAction *a, _actions->toolActions()->actions())
    _toolmenu->addAction(a);

  QDesignerComponents::initializeResources();

  _widgetwindow   = new WidgetBoxWindow(this);
  _objinspwindow  = new ObjectInspectorWindow(this);
  _propinspwindow = new PropertyEditorWindow(this);
  _slotedwindow   = new SignalSlotEditorWindow(this);

  connect(_actions->widgetBoxAct(), SIGNAL(toggled(bool)), _widgetwindow,  SLOT(setVisible(bool)));
  connect(_actions->objectInspAct(),SIGNAL(toggled(bool)), _objinspwindow, SLOT(setVisible(bool)));
  connect(_actions->propertyEdAct(),SIGNAL(toggled(bool)), _propinspwindow,SLOT(setVisible(bool)));
  connect(_actions->signalSlotAct(),SIGNAL(toggled(bool)), _slotedwindow,  SLOT(setVisible(bool)));

  omfgThis->handleNewWindow(_widgetwindow);
  omfgThis->handleNewWindow(_objinspwindow);
  omfgThis->handleNewWindow(_propinspwindow);
  omfgThis->handleNewWindow(_slotedwindow);

  // resource editor;
  // action editor;

  _integration = new qdesigner_internal::QDesignerIntegration(_formeditor, this);

  // toolbar creation

  _formwindow = _formeditor->formWindowManager()->createFormWindow(this, Qt::Window);
  qDebug() << "_formwindow->hasFeature(EditFeature) = " << _formwindow->hasFeature(QDesignerFormWindowInterface::EditFeature);
  _formeditor->formWindowManager()->setActiveFormWindow(_formwindow);
  _formeditor->objectInspector()->setFormWindow(_formwindow);
  _formwindow->editWidgets();

  QWidget *placeholder = new QWidget(this);
  setCentralWidget(placeholder);
  QHBoxLayout *editorLayout = new QHBoxLayout(placeholder);
  editorLayout->addWidget(_formwindow);

  _designer = _formeditor->topLevel();
  if (_designer)
  {
    _designer->setObjectName("_designer");
    omfgThis->handleNewWindow(_designer);
  }
}

xTupleDesigner::~xTupleDesigner()
{
  if (_objinspwindow)  _objinspwindow->close();
  if (_propinspwindow) _propinspwindow->close();
  if (_slotedwindow)   _slotedwindow->close();
  if (_widgetwindow)   _widgetwindow->close();

  if (_designer)   _designer->deleteLater();
  if (_formeditor) _formeditor->deleteLater();
  if (_formwindow) _formwindow->deleteLater();
}

QString xTupleDesigner::name()
{
  QDomDocument xmldoc("UIFile");
  _source->reset();
  xmldoc.setContent(_source);
  QDomNode classnode = xmldoc.firstChild().firstChild();
  if (classnode.isNull())
  {
    qWarning("xTupleDesigner::name() classnode is null");
    return QString();
  }

  if (classnode.toElement().tagName() != "class")
  {
    qWarning("xTupleDesigner::name() first child %s, not a class element",
           qPrintable(classnode.toElement().tagName() ));
    return QString();
  }

  return classnode.toElement().text();
}

void xTupleDesigner::setFormEnabled(bool p)
{
  _formEnabled = p;
  emit formEnabledChanged(p);
}

void xTupleDesigner::setFormId(int p)
{
  _formId = p;
  emit formIdChanged(p);
}

void xTupleDesigner::setNotes(QString p)
{
  _notes = p;
  emit notesChanged(p);
}

void xTupleDesigner::setOrder(int p)
{
  _order = p;
  emit orderChanged(p);
}

static void debugObjectTree(QObject *p, QString label, int depth)
{
  for (int i = 0; i < p->children().size(); i++)
  {
    QObject *child = p->children().at(i);
    qDebug() << label << "," << QString::number(i) << ":"
             << child
             << (QString(child->inherits("QWidget") ? "is" : "not") + " a widget")
             << (child->inherits("QWidget") ?
                   QString("that ") +
                   (((QWidget*)child)->acceptDrops() ? "" : "cannot ") +
                   "accept drops" : "");
    if (depth > 0)
      debugObjectTree(child, label + "," + QString::number(i), depth - 1);
  }
}

void xTupleDesigner::setSource(QIODevice *psrc, QString pfilename)
{
  if (! psrc || ! _formwindow)
    return;

  if (! psrc->open(QIODevice::ReadOnly | QIODevice::Text))
  {
    qWarning("could not open .UI");
    return;
  }
  _source = psrc;
  _formwindow->setContents(_source);
  _source->reset();
  _formwindow->setFileName(pfilename);
  emit nameChanged(name());
  emit sourceChanged(_formwindow->contents());

  debugObjectTree(this, "setSource(QIODevice,QString)", 3);
}

void xTupleDesigner::setSource(QString psrc)
{
  _source->close();
  // TODO: memory leak?
  _source = new QBuffer(new QByteArray(psrc.toAscii()), this);
  _source->open(QIODevice::ReadOnly | QIODevice::Text);
  _formwindow->setContents(_source);
  _source->reset();
  _formwindow->setFileName("");
  emit nameChanged(name());
  emit sourceChanged(_formwindow->contents());

  qDebug() << "setSource(QString);";
  for (int i = 0; i < children().size(); i++)
    qDebug() << "setSource() " << i << ": " << children().at(i);
}

QString xTupleDesigner::source()
{
  return _formwindow->contents();
}

// TODO: this always seems to return true
bool xTupleDesigner::isChanged()
{
  _source->reset();
  bool returnValue =
          (_source->size()    != _formwindow->contents().size() ||
           _source->readAll().data() != _formwindow->contents().toAscii().data());
  _source->reset();
  qDebug("xTupleDesigner::isChanged() returning %d", returnValue);
  return returnValue;
}
