/*THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.*/
#include <QFileDialog>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QMessageBox>
#include <QPushButton>
#include <QDebug>
#include <QStringBuilder>
#include <QSettings>
//#include <iostream>
#include "debbin.h"
#include "ui_debbin.h"

//using namespace std;

QProcess * dpkg = 0;

debbin::debbin(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::debbin)
{
    ui->setupUi(this);
    QDir d;
    d.mkdir("configs"); // ensure there is a default directory for the configs dialog
}

debbin::~debbin()
{
    delete ui;
}

void debbin::on_checkBox_toggled(bool checked)
{
    ui->ln_dependancies->setEnabled(checked);
}

void debbin::on_pushButton_clicked()
{
    QMessageBox msg;
    const QString package = ui->ln_projectname->text().trimmed().replace(" ", "_");
    QString version = ui->ln_version->text().trimmed();
    if(!version.contains("-"))
    {
        version = version + "-0";
    }
    QString depends;
    const QString maintainer = ui->ln_maintainer->text().trimmed();
    const QString desc_title = ui->ln_descriptiontitle->text().trimmed();
    const QString desc_body = ui->ln_description->toPlainText().trimmed();
    QString desc;
    QString arch = ui->ln_architecture->text().trimmed();
    if (arch.isEmpty())
    {
        arch = "all";
    }
    const QString deps = ui->ln_dependancies->text().trimmed();
    if (ui->checkBox->isChecked() && (deps != ""))
    {
        depends = "\ndepends: " % deps;
    }
    else
    {
        depends = "";
    }

    desc = "\ndescription: " % desc_title;
    if (desc_body != "")
    {
        desc.append("\n             " % desc_body);
    }
    ui->txt_control->setPlainText("package: " % package % "\nversion: " % version % "\narchitecture: " % arch % depends % "\nmaintainer: " % maintainer % desc % "\n");
    ui->btn_createpackage->setEnabled(true);
}

void debbin::on_btn_createpackage_clicked()
{
    dpkg = new QProcess(this);
    connect(dpkg, &QProcess::readyRead, this, &debbin::on_readyRead);
    const QString path = ui->ln_filesystem->text().trimmed();
    QDir debian_dir(path % "/DEBIAN/");
    debian_dir.mkdir(path % "/DEBIAN/");
    QFile control_file(path % "/DEBIAN/control");

    control_file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&control_file);

    out << ui->txt_control->toPlainText();
    out << "\n";

    control_file.flush();
    control_file.close();

    const QString outpath = ui->ln_outputdir->text().trimmed();
    //qDebug() << "dpkg-deb --build " % path % " " % outpath;
    dpkg->start("dpkg-deb", QStringList()<<"--build"<<path<<outpath, QIODevice::ReadWrite);
    ui->txt_output->setText("dpkg-deb --build " % path % " " % outpath);
    //qDebug()<<dpkg.arguments();
//    QByteArray data;

//    while(dpkg.waitForReadyRead())
//    {
//        data.append(dpkg.readAll());
//    }

//    ui->txt_output->setText(data);
}

void debbin::on_readyRead()
{
    //qDebug()<<dpkg->readAll();
    ui->txt_output->setText(ui->txt_output->text().append(dpkg->readAll()).append("\n"));
}

void debbin::on_btn_filesystem_clicked()
{
    QString dirName = QFileDialog::getExistingDirectory(this, "Choose Location of Build Files Directory");
    ui->ln_filesystem->setText(dirName);
}

void debbin::on_btn_outbutdir_clicked()
{
    QString filePath = QFileDialog::getExistingDirectory(this, "Choose the Location for the Output File");
    ui->ln_outputdir->setText(filePath);
}

void debbin::on_actionQuit_triggered()
{
    QApplication::quit();
}

void debbin::on_actionGenerate_control_file_triggered()
{
    debbin::on_pushButton_clicked();
}

void debbin::on_actionCreate_package_triggered()
{
    debbin::on_btn_createpackage_clicked();
}

void debbin::on_actionAbout_triggered()
{
    QMessageBox::information(this, "About", " About debbin\n\ndebbin is a simple application designed to make the creation of debian packages for binary distributions easier. \
Provides a straight forward graphical user interface and to generate control files and debian packages without the need for any command \
line knowledge.\n\nOriginal source via Ben Heidemann's deb-creator on SourceForge.\n\ndebbin developer, Mitch Winkle.\n\nNO WARRANTY is expressed or implied.");
}

void debbin::on_btn_add_clicked()
{
    ui->txt_control->setPlainText(ui->txt_control->toPlainText() % ui->cmb_condition->currentText() % ": " % ui->ln_value->text() % "\n");
    if(s_currentConfig.isEmpty())
        return;
    QSettings s(s_currentConfig, QSettings::IniFormat);
    const QString c(ui->cmb_condition->currentText().trimmed());
    s.beginGroup(c);
    s.remove(""); // clear for a new value
    s.setValue(c, ui->ln_value->text().trimmed());
    s.endGroup();
    QStringList cmb;
    for (int i = 0; i < ui->cmb_condition->count(); i++)
        cmb<<ui->cmb_condition->currentText();
    s.setValue("cmb", cmb);
}


void debbin::on_delButton_clicked()
{
    ui->ln_value->clear();
    QSettings s(s_currentConfig, QSettings::IniFormat);
    QStringList cmb = s.value("cmb", "").toStringList();
    const QString c(ui->cmb_condition->currentText().trimmed());
    s.beginGroup(c);
    s.remove(""); // clear this group
    s.endGroup();
    cmb.removeAt(ui->cmb_condition->currentIndex());
    s.value("cmb", cmb);
    ui->cmb_condition->removeItem(ui->cmb_condition->currentIndex());
    on_cmb_condition_activated(ui->cmb_condition->currentText());

}

void debbin::on_actionUser_manual_triggered()
{
    QMessageBox::information(this, "User Manual", "The user manual does not exist.");
}

void debbin::on_actionClear_triggered()
{
    ui->txt_output->clear();
}

void debbin::on_action_Open_Config_triggered()
{
    const QString f = QFileDialog::getOpenFileName(this, "Choose .deb Config File", "./configs", "*.ini");
    s_currentConfig = f;
    if(f.isEmpty())
        return;
    readConfig(f);
    ui->btn_createpackage->setEnabled(false);
}

void debbin::readConfig(const QString file)
{
    QSettings s(file, QSettings::IniFormat);
    ui->ln_projectname->setText(s.value("projectname", "").toString());
    ui->ln_maintainer->setText(s.value("maintainer", "").toString());
    ui->ln_version->setText(s.value("version", "").toString());
    ui->ln_architecture->setText(s.value("architecture", "").toString());
    ui->ln_dependancies->setText(s.value("dependencies", "").toString());
    ui->ln_filesystem->setText(s.value("filesystem", "").toString());
    ui->ln_outputdir->setText(s.value("outputdir", "").toString());
    ui->ln_descriptiontitle->setText(s.value("descriptiontitle", "").toString());
    ui->ln_description->setPlainText(s.value("description", "").toString());
    const QStringList cmb = s.value("cmb", "").toStringList();
    ui->cmb_condition->clear();
    ui->cmb_condition->addItems(cmb);
    const QString first = ui->cmb_condition->itemText(0);
    if(first.isEmpty())
        return;
    s.beginGroup(first);
    // cmb values should have exactly one key/value pair
    // so it matters not what the key name is
    ui->ln_value->setText(s.value(s.childKeys().at(0), "").toString());
    s.endGroup();
}

void debbin::on_saveConfigButton_clicked()
{
    on_action_Save_Config_triggered();
}

void debbin::on_action_Save_Config_triggered()
{
    // the user supplied file name will be used to save the config
    const QString out = QFileDialog::getSaveFileName(this, "Save Config", "Choose a filename to which to save this configuration.", "*.ini");
    if(out.isEmpty())
        return;
    QSettings s(out, QSettings::IniFormat);
    s.setValue("projectname", ui->ln_projectname->text().trimmed());
    s.setValue("maintainer", ui->ln_maintainer->text().trimmed());
    s.setValue("version", ui->ln_version->text().trimmed());
    s.setValue("architecture", ui->ln_architecture->text().trimmed());
    s.setValue("dependencies", ui->ln_dependancies->text().trimmed());
    s.setValue("filesystem", ui->ln_filesystem->text().trimmed());
    s.setValue("outputdir", ui->ln_outputdir->text().trimmed());
    s.setValue("descriptiontitle", ui->ln_descriptiontitle->text().trimmed());
    s.setValue("description", ui->ln_description->toPlainText());
    QStringList items;
    for(int i = 0; i < ui->cmb_condition->count(); i++)
        items<<ui->cmb_condition->itemText(i);
    s.setValue("cmb", items);
}

void debbin::on_cmb_condition_activated(const QString &arg1)
{
    if (s_currentConfig.isEmpty())
        return;
    QSettings s(s_currentConfig, QSettings::IniFormat);
    const QStringList cmb = s.value("cmb", "").toStringList();
    if(cmb.isEmpty() || !cmb.contains(arg1))
        return;
    s.beginGroup(arg1);
    ui->ln_value->setText(s.value(s.childKeys().at(0), "").toString());
    s.endGroup();
}

