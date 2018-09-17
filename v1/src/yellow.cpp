/*
* Cyan <http://cyan.fxarena.net> <https://github.com/olear/cyan>,
* Copyright (C) 2016, 2017 Ole-André Rodlie
*
* Cyan is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as published
* by the Free Software Foundation.
*
* Cyan is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Cyan.  If not, see <http://www.gnu.org/licenses/gpl-2.0.html>
*/

#include "yellow.h"
#include <QDirIterator>
#include <QDir>
#include <QFile>
#include <QSettings>

Yellow::Yellow(QObject *parent) :
    QObject(parent)
{
}

Yellow::~Yellow()
{
}

QString Yellow::profileDescFromFile(QString file)
{
    QString output;
    QFileInfo profile(file);
    if (profile.suffix().contains(QRegExp("(icc|icm)",Qt::CaseInsensitive)) && profile.exists()) {
        cmsHPROFILE lcmsProfile;
        lcmsProfile = cmsOpenProfileFromFile(file.toUtf8(), "r");
        if (lcmsProfile) {
            cmsUInt32Number size = 0;
            size = cmsGetProfileInfoASCII(lcmsProfile, cmsInfoDescription, "en", "US", NULL, 0);
            if (size > 0) {
                char buffer[size+1];
                size = cmsGetProfileInfoASCII(lcmsProfile, cmsInfoDescription, "en", "US", buffer, size);
                if (size > 0) {
                    output=QString::fromUtf8(buffer);
                }
            }
        }
        cmsCloseProfile(lcmsProfile);
    }
    return output;
}

QString Yellow::profileDescFromData(QByteArray data)
{
    QString output;
    if (data.length() > 0) {
        cmsHPROFILE lcmsProfile;
        lcmsProfile = cmsOpenProfileFromMem(data.data(), data.length());
        if (lcmsProfile) {
            cmsUInt32Number size = 0;
            size = cmsGetProfileInfoASCII(lcmsProfile, cmsInfoDescription, "en", "US", NULL, 0);
            if (size > 0) {
                char buffer[size+1];
                size = cmsGetProfileInfoASCII(lcmsProfile, cmsInfoDescription, "en", "US", buffer, size);
                if (size > 0) {
                    output=QString::fromUtf8(buffer);
                }
            }
        }
        cmsCloseProfile(lcmsProfile);
    }
    return output;
}

QString Yellow::profileManufacturerFromFile(QString file)
{
    QString output;
    QFileInfo profile(file);
    if (profile.suffix().contains(QRegExp("(icc|icm)",Qt::CaseInsensitive)) && profile.exists()) {
        cmsHPROFILE lcmsProfile;
        lcmsProfile = cmsOpenProfileFromFile(file.toUtf8(), "r");
        if (lcmsProfile) {
            cmsUInt32Number size = 0;
            size = cmsGetProfileInfoASCII(lcmsProfile, cmsInfoManufacturer, "en", "US", NULL, 0);
            if (size > 0) {
                char buffer[size+1];
                size = cmsGetProfileInfoASCII(lcmsProfile, cmsInfoManufacturer, "en", "US", buffer, size);
                if (size > 0) {
                    output=QString::fromUtf8(buffer);
                }
            }
        }
        cmsCloseProfile(lcmsProfile);
    }
    return output;
}

QString Yellow::profileCopyrightFromFile(QString file)
{
    QString output;
    QFileInfo profile(file);
    if (profile.suffix().contains(QRegExp("(icc|icm)",Qt::CaseInsensitive)) && profile.exists()) {
        cmsHPROFILE lcmsProfile;
        lcmsProfile = cmsOpenProfileFromFile(file.toUtf8(), "r");
        if (lcmsProfile) {
            cmsUInt32Number size = 0;
            size = cmsGetProfileInfoASCII(lcmsProfile, cmsInfoCopyright, "en", "US", NULL, 0);
            if (size > 0) {
                char buffer[size+1];
                size = cmsGetProfileInfoASCII(lcmsProfile, cmsInfoCopyright, "en", "US", buffer, size);
                if (size > 0) {
                    output=QString::fromUtf8(buffer);
                }
            }
        }
        cmsCloseProfile(lcmsProfile);
    }
    return output;
}

int Yellow::profileColorSpaceFromFile(QString file)
{
    int status = 0;
    QFileInfo iccFile(file);
    if (iccFile.suffix().contains(QRegExp("(icc|icm)", Qt::CaseInsensitive)) && iccFile.exists()) {
        cmsHPROFILE lcmsProfile;
        lcmsProfile = cmsOpenProfileFromFile(file.toUtf8(), "r");
        if (lcmsProfile) {
            if (cmsGetColorSpace(lcmsProfile) == cmsSigRgbData) {
                status = 1;
            } else if (cmsGetColorSpace(lcmsProfile) == cmsSigCmykData) {
                status = 2;
            } else if (cmsGetColorSpace(lcmsProfile) == cmsSigGrayData) {
                status = 3;
            }
        }
        cmsCloseProfile(lcmsProfile);
    }
    return status;
}

int Yellow::profileColorSpaceFromData(QByteArray data)
{
    int status = 0;
    if (data.length() > 0) {
        cmsHPROFILE lcmsProfile;
        lcmsProfile = cmsOpenProfileFromMem(data.data(), data.length());
        if (lcmsProfile) {
            if (cmsGetColorSpace(lcmsProfile) == cmsSigRgbData) {
                status = RGB_COLORSPACE;
            } else if (cmsGetColorSpace(lcmsProfile) == cmsSigCmykData) {
                status = CMYK_COLORSPACE;
            } else if (cmsGetColorSpace(lcmsProfile) == cmsSigGrayData) {
                status = GRAY_COLORSPACE;
            }
        }
        cmsCloseProfile(lcmsProfile);
    }
    return status;
}

QStringList Yellow::genProfiles(int colorspace)
{
    QStringList output;
    QStringList folders;
    folders << QDir::rootPath() + "/WINDOWS/System32/spool/drivers/color";
    folders << "/Library/ColorSync/Profiles";
    folders << QDir::homePath() + "/Library/ColorSync/Profiles";
    folders << "/usr/share/color/icc";
    folders << "/usr/local/share/color/icc";
    folders << QDir::homePath() + "/.color/icc";
    QString cyanICCPath = QDir::homePath() + "/.config/Cyan/icc";
    QDir cyanICCDir(cyanICCPath);
    if (cyanICCDir.exists(cyanICCPath)) {
        folders << cyanICCPath;
    }
    for (int i = 0; i < folders.size(); ++i) {
        QStringList filter;
        filter << "*.icc";
        filter << "*.icm";
        QDirIterator it(folders.at(i), filter, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString iccFile = it.next();
            QString profile = profileDescFromFile(iccFile);
            int profileColor = profileColorSpaceFromFile(iccFile);
            if (profileColor==colorspace && !profile.isEmpty()) {
                output << iccFile + "|" + profile;
            }
        }
    }
    output.removeDuplicates();
    return output;
}

bool Yellow::editProfile(QString file, QString output, QString description, QString copyright)
{
    bool result = false;
    QFileInfo profile(file);
    if (profile.suffix().contains(QRegExp("(icc|icm)",Qt::CaseInsensitive)) && profile.exists()) {
        cmsHPROFILE lcmsProfile;
        QByteArray data;
        QFile inFile(file);
        if (inFile.open(QIODevice::ReadOnly|QIODevice::Text)) {
            data = inFile.readAll();
            inFile.close();
        }
        if (data.length()==0) {
            return false;
        }
        lcmsProfile = cmsOpenProfileFromMem(data.data(), data.length());
        if (lcmsProfile) {
            cmsContext ContextID = cmsGetProfileContextID(lcmsProfile);
            bool modified = false;
            if (!description.isEmpty()) {
                cmsMLU *DescriptionMLU;
                DescriptionMLU  = cmsMLUalloc(ContextID, 1);
                if (!cmsMLUsetWide(DescriptionMLU,  "en", "US", description.toStdWString().c_str())) {
                    result = false;
                }
                if (cmsWriteTag(lcmsProfile, cmsSigProfileDescriptionTag,  DescriptionMLU)) {
                    modified = true;
                }
                if (DescriptionMLU) {
                    cmsMLUfree(DescriptionMLU);
                }
            }
            if (!copyright.isEmpty()) {
                cmsMLU *CopyrightMLU;
                CopyrightMLU  = cmsMLUalloc(ContextID, 1);
                if (!cmsMLUsetWide(CopyrightMLU,  "en", "US", copyright.toStdWString().c_str())) {
                    result = false;
                }
                if (cmsWriteTag(lcmsProfile, cmsSigCopyrightTag,  CopyrightMLU)) {
                    modified = true;
                }
                if (CopyrightMLU) {
                    cmsMLUfree(CopyrightMLU);
                }
            }
            if (modified) {
                if (cmsSaveProfileToFile(lcmsProfile, output.toUtf8().data())) {
                    result = true;
                }
            }
        }
        cmsCloseProfile(lcmsProfile);
    }
    return result;
}

QByteArray Yellow::profileDefault(int colorspace)
{
    QByteArray bytes;
    if (colorspace>0) {
        QString fileName;
        QSettings settings;
        settings.beginGroup("profiles");
        if (!settings.value(QString::number(colorspace)).isNull()) {
            fileName = settings.value(QString::number(colorspace)).toString();
        }
        settings.endGroup();
        if (!fileName.isEmpty()) {
            QFile proFile(fileName);
            if (proFile.exists()) {
                if (proFile.open(QIODevice::ReadOnly)) {
                    bytes = proFile.readAll();
                    proFile.close();
                }
            }
        }
    }
    return bytes;
}