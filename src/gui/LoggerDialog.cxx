//============================================================================
//
//   SSSS    tt          lll  lll
//  SS  SS   tt           ll   ll
//  SS     tttttt  eeee   ll   ll   aaaa
//   SSSS    tt   ee  ee  ll   ll      aa
//      SS   tt   eeeeee  ll   ll   aaaaa  --  "An Atari 2600 VCS Emulator"
//  SS  SS   tt   ee      ll   ll  aa  aa
//   SSSS     ttt  eeeee llll llll  aaaaa
//
// Copyright (c) 1995-2020 by Bradford W. Mott, Stephen Anthony
// and the Stella Team
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//============================================================================

#include "bspf.hxx"
#include "Dialog.hxx"
#include "FSNode.hxx"
#include "GuiObject.hxx"
#include "OSystem.hxx"
#include "FrameBuffer.hxx"
#include "Settings.hxx"
#include "PopUpWidget.hxx"
#include "StringListWidget.hxx"
#include "StringParser.hxx"
#include "Widget.hxx"
#include "Font.hxx"
#include "Logger.hxx"
#include "LoggerDialog.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
LoggerDialog::LoggerDialog(OSystem& osystem, DialogContainer& parent,
                           const GUI::Font& font, int max_w, int max_h,
                           bool uselargefont)
  : Dialog(osystem, parent, font, "System logs")
{
  const int lineHeight   = font.getLineHeight(),
            buttonWidth  = font.getStringWidth("Save log to disk") + 20,
            buttonHeight = font.getLineHeight() + 4;
  int xpos, ypos;
  WidgetArray wid;

  // Set real dimensions
  // This is one dialog that can take as much space as is available
  setSize(4000, 4000, max_w, max_h);

  // Test listing of the log output
  xpos = 10;  ypos = 10 + _th;
  myLogInfo = new StringListWidget(this, uselargefont ? font :
                  instance().frameBuffer().infoFont(), xpos, ypos, _w - 2 * xpos,
                  _h - buttonHeight - ypos - 20 - 2 * lineHeight, false);
  myLogInfo->setEditable(false);
  wid.push_back(myLogInfo);
  ypos += myLogInfo->getHeight() + 8;

  // Level of logging (how much info to print)
  VariantList items;
  VarList::push_back(items, "None", static_cast<int>(Logger::Level::ERR));
  VarList::push_back(items, "Basic", static_cast<int>(Logger::Level::INFO));
  VarList::push_back(items, "Verbose", static_cast<int>(Logger::Level::DEBUG));
  myLogLevel =
    new PopUpWidget(this, font, xpos, ypos, font.getStringWidth("Verbose"),
                    lineHeight, items, "Log level ",
                    font.getStringWidth("Log level "));
  wid.push_back(myLogLevel);

  // Should log output also be shown on the console?
  xpos += myLogLevel->getWidth() + 32;
  myLogToConsole = new CheckboxWidget(this, font, xpos, ypos + 1, "Print to console");
  wid.push_back(myLogToConsole);

  // Add Save, OK and Cancel buttons
  ButtonWidget* b;
  b = new ButtonWidget(this, font, 10, _h - buttonHeight - 10,
                       buttonWidth, buttonHeight, "Save log to disk",
                       GuiObject::kDefaultsCmd);
  wid.push_back(b);
  addOKCancelBGroup(wid, font);

  addToFocusList(wid);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void LoggerDialog::loadConfig()
{
  StringParser parser(Logger::instance().logMessages());
  myLogInfo->setList(parser.stringList());
  myLogInfo->setSelected(0);
  myLogInfo->scrollToEnd();

  myLogLevel->setSelected(instance().settings().getString("loglevel"),
    static_cast<int>(Logger::Level::INFO));
  myLogToConsole->setState(instance().settings().getBool("logtoconsole"));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void LoggerDialog::saveConfig()
{
  int loglevel = myLogLevel->getSelectedTag().toInt();
  bool logtoconsole = myLogToConsole->getState();

  instance().settings().setValue("loglevel", loglevel);
  instance().settings().setValue("logtoconsole", logtoconsole);

  Logger::instance().setLogParameters(loglevel, logtoconsole);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void LoggerDialog::saveLogFile()
{
  ostringstream path;
  path << instance().defaultSaveDir() << "stella.log";
  FilesystemNode node(path.str());

  ofstream out(node.getPath());
  if(out.is_open())
  {
    out << Logger::instance().logMessages();
    instance().frameBuffer().showMessage("Saving log file to " + path.str());
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void LoggerDialog::handleCommand(CommandSender* sender, int cmd,
                                 int data, int id)
{
  switch(cmd)
  {
    case GuiObject::kOKCmd:
      saveConfig();
      close();
      break;

    case GuiObject::kDefaultsCmd:
      saveLogFile();
      break;

    default:
      Dialog::handleCommand(sender, cmd, data, 0);
      break;
  }
}
