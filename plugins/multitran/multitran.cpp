/******************************************************************************
 * This file is part of the Mula project
 * Copyright (c) 2004 Stanislav Ievlev
 * Copyright (c) 2008 Nick Shaforostoff
 * Copyright (c) 2011 Laszlo Papp <lpapp@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "multitran.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QTextCodec>

#include <facet/identification.hh>
#include <facet/alphabet.hh>
#include <facet/typographic.hh>
#include <mt/query/linguas.hh>
#include <mt/query/translation.hh>
#include <mt/query/singleton.hh>
#include <mt/query/config.hh>
#include <mt/query/file_map.hh>
#include <mt/support/path.hh>
#include <mt/query/lgk.hh>
#include <mt/support/str.hh>
#include <mt/query/lang_info.hh>

#include <algorithm>
#include <functional>
#include <memory>

#include <iostream>
#include <stdexcept>
#include <vector>
#include <string>

typedef mt::singleton_array<mt::file_map> txtdb_type;

struct compare_names
{
    compare_names(const std::string& from,const std::string& to):
        from_(from),
        to_(to)
    {}
    bool operator()(const mt::lang_pair& lng1,const mt::lang_pair& lng2)
    {
        return distance(lng1) < distance(lng2);
    }
    int distance(const mt::lang_pair& lng)
    {
        std::string from_name=mt::lang_name(lng.first);
        std::string to_name=mt::lang_name(lng.second);

        return (!from_.empty() && !from_name.compare(0,from_.size(),from_)) +
                (!to_.empty() && !to_name.compare(0,to_.size(),to_));
    }
    std::string from_,to_;
};

int compare_articles(const mt::article& a1,const mt::article& a2)
{
    if (a1.lgk() != a2.lgk())
        return a2.lgk() > a1.lgk();
    else
        return a2.subject() > a1.subject();
}

struct show
{
    show(std::string& r_, bool& found_): r(r_),found(found_) {}
    void operator()(mt::article_set as)
    {
        mt::file_map& subj = txtdb_type::instance(mt::datapath+mt::path_separator()+"subjects.txt");
        mt::file_map& spart = txtdb_type::instance(mt::datapath+mt::path_separator()+"speechparts.txt");

        if (!as.articles_.empty())
        {
            found=true;
            std::sort(as.articles_.begin(),as.articles_.end(),compare_articles);

            int prev_lgk = -1;
            std::string prev_subject = "x";
            for(size_t i=0;i<as.articles_.size();++i)
            {
                const mt::article& a = as.articles_[i];
                if (prev_lgk != a.lgk())
                {
                    r+="<tr><td><b>"+a.orig()+","+
                    spart.any_name(mt::to_string<int>(mt::speech_part(a.lgk())))+"</b></td></tr>";
                    prev_lgk = a.lgk();
                    prev_subject = "x";//reset subject
                }
                if (prev_subject != a.subject())
                {
                    r+="<tr><td></td><td><font class=\"explanation\">";
                    r+=subj.any_name(a.subject());
                    r+="</font></td><td>";
                    r+=a.translated();
                    prev_subject = a.subject();
                }
                else
                    r+=", "+a.translated();
            }
            r+="</td></tr>";
        }
    }
    std::string &r;
    bool& found;
};

std::string do_translate(const std::string& text,mt::lang_code from,mt::lang_code to)
{
    bool found=false;
    std::string r="<table>";
    mt::phrase ph;
    mt::fill_phrase(ph,text,from);
    mt::translation tr(ph,from,to);
    std::for_each(tr.asets().begin(), tr.asets().end(), show(r,found));
    r+="</table>";
    if (found)
        return r;
    return "";
}

class MultiTran::Private
{
    public:
        Private()
        {   
        }

        ~Private()
        {
        }

        QStringList dictionaryDirs;
        QHash<QString, int> loadedDicts;
}

MultiTran::MultiTran(QObject *parent)
    : QObject(parent)
{
//     QSettings settings("qstardict","qstardict");
//     m_dictDirs = settings.value("MultiTran/dictDirs", m_dictDirs).toStringList();
//     m_reformatLists = settings.value("MultiTran/reformatLists", true).toBool();
}

MultiTran::~MultiTran()
{
//     QSettings settings("qstardict","qstardict");
//     settings.setValue("MultiTran/dictDirs", m_dictDirs);
//     settings.setValue("MultiTran/reformatLists", m_reformatLists);
}

QString MultiTran::name() const
{
    return "multitran";
}

QString MultiTran::version() const
{
    return "0.1";
}

QString MultiTran::description() const
{
    return "The MultiTran plugin";
}

QStringList MultiTran::authors() const
{
    return QStringList() << tr("Stanislav Ievlev <inger@altlinux.org>") << tr("Nick Shaforostoff <shaforostoff@kde.ru>");
}

Features MultiTran::features() const
{
    return Features(/*SearchSimilar | SettingsDialog*/);
}

QStringList MultiTran::availableDicts() const
{
    return QStringList("MultiTran");
}

void MultiTran::setLoadedDicts(const QStringList &loadedDicts)
{
}

MultiTran::DictInfo MultiTran::dictInfo(const QString &dict)
{
//     ::DictInfo nativeInfo;
//     nativeInfo.wordcount = 0;

    DictInfo result(name(), dict);
    result.setAuthor("Multitran.ru");
    result.setDescription(tr("1 mln words excerpt of multitran.ru"));
    result.setWordsCount(-1);
    return result;
}

bool MultiTran::isTranslatable(const QString &dict, const QString &word)
{
    return true;
}

MultiTran::Translation MultiTran::translate(const QString &dict, const QString &word)
{
    QTextCodec* c=QTextCodec::codecForMib(2251);
    std::string text=c->fromUnicode(word).data();
    std::string from_lang,to_lang;

    int i=word.size();
    while(--i>=0)
        if (word.at(i).unicode()>127)
            break;

    if (i!=-1)
        from_lang="russian";
    else
        from_lang="english";

    mt::linguas avail_langs;
    mt::linguas::iterator lang = std::max_element(avail_langs.begin(),
                                                avail_langs.end(),
                                                compare_names(from_lang,to_lang));
    if (lang == avail_langs.end() ||
    (!from_lang.empty() && !to_lang.empty() && (compare_names(from_lang,to_lang).distance(*lang)!=2)))
    {
        //std::cerr<<"illegal language names"<<std::endl;
        return Translation();
    }

    //"<hr width=50%><center><b>multitran</b><center><hr width=50%>";
    QString queryResult=c->toUnicode(do_translate(lower_str(lang->first,text),
                        lang->first,lang->second).c_str());

    if (queryResult.isEmpty())
        return Translation();

    return Translation(word,"MultiTran",queryResult);
}

QStringList MultiTran::findSimilarWords(const QString &dict, const QString &word)
{
    return QStringList();
}

int MultiTran::execSettingsDialog(QWidget *parent)
{
    //::SettingsDialog dialog(this, parent);
    //return dialog.exec();
    return 0;
}


Q_EXPORT_PLUGIN2(multitran, MultiTran)
