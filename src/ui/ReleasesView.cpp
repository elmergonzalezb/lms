/*
 * Copyright (C) 2018 Emeric Poupon
 *
 * This file is part of LMS.
 *
 * LMS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * LMS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with LMS.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <Wt/WApplication>
#include <Wt/WAnchor>
#include <Wt/WImage>
#include <Wt/WText>
#include <Wt/WTemplate>

#include "database/Types.hpp"

#include "utils/Logger.hpp"
#include "utils/Utils.hpp"

#include "LmsApplication.hpp"
#include "Filters.hpp"
#include "ReleasesView.hpp"

namespace UserInterface {

using namespace Database;

Releases::Releases(Filters* filters, Wt::WContainerWidget* parent)
: Wt::WContainerWidget(parent),
	_filters(filters)
{
	auto container = new Wt::WTemplate(Wt::WString::tr("template-releases"), this);
	container->addFunction("tr", &Wt::WTemplate::Functions::tr);

	_search = new Wt::WLineEdit();
	container->bindWidget("search", _search);
	_search->setPlaceholderText(Wt::WString::tr("msg-search-placeholder"));
	_search->textInput().connect(this, &Releases::refresh);

	_releasesContainer = new Wt::WContainerWidget();
	container->bindWidget("releases", _releasesContainer);

	_showMore = new Wt::WTemplate(Wt::WString::tr("template-show-more"));
	_showMore->addFunction("tr", &Wt::WTemplate::Functions::tr);
	container->bindWidget("show-more", _showMore);

	_showMore->clicked().connect(std::bind([=]
	{
		addSome();
	}));

	refresh();

	filters->updated().connect(this, &Releases::refresh);
}

void
Releases::refresh()
{
	_releasesContainer->clear();
	addSome();
}

void
Releases::addSome()
{
	auto searchKeywords = splitString(_search->text().toUTF8(), " ");

	auto clusterIds = _filters->getClusterIds();

	Wt::Dbo::Transaction transaction(DboSession());

	bool moreResults;
	auto releases = Release::getByFilter(DboSession(), clusterIds, searchKeywords, _releasesContainer->count(), 20, moreResults);

	for (auto release : releases)
	{
		auto releaseId = release.id();

		Wt::WTemplate* entry = new Wt::WTemplate(Wt::WString::tr("template-releases-entry"), _releasesContainer);
		entry->addFunction("tr", Wt::WTemplate::Functions::tr);

		Wt::WAnchor* coverAnchor = new Wt::WAnchor(Wt::WLink(Wt::WLink::InternalPath, "/release/" + std::to_string(releaseId)));
		Wt::WImage* cover = new Wt::WImage(coverAnchor);
		cover->setImageLink(SessionImageResource()->getReleaseUrl(releaseId, 128));
		// Some images may not be square
		cover->setWidth(128);
		entry->bindWidget("cover", coverAnchor);

		{
			Wt::WAnchor* releaseAnchor = new Wt::WAnchor(Wt::WLink(Wt::WLink::InternalPath, "/release/" + std::to_string(releaseId)));
			Wt::WText* releaseName = new Wt::WText(releaseAnchor);
			releaseName->setText(Wt::WString::fromUTF8(release->getName(), Wt::PlainText));
			entry->bindWidget("release-name", releaseAnchor);
		}


		auto artists = release->getArtists();
		if (artists.size() > 1)
		{
			entry->bindString("artist-name", Wt::WString::tr("msg-various-artists"));
		}
		else
		{
			Wt::WAnchor* artistAnchor = new Wt::WAnchor(Wt::WLink(Wt::WLink::InternalPath, "/artist/" + std::to_string(artists.front().id())));
			Wt::WText* artist = new Wt::WText(artistAnchor);
			artist->setText(Wt::WString::fromUTF8(artists.front()->getName(), Wt::PlainText));
			entry->bindWidget("artist-name", artistAnchor);
		}

		auto playBtn = new Wt::WText(Wt::WString::tr("btn-releases-play-btn"), Wt::XHTMLText);
		entry->bindWidget("play-btn", playBtn);
		playBtn->clicked().connect(std::bind([=]
		{
			releasePlay.emit(releaseId);
		}));

		auto addBtn = new Wt::WText(Wt::WString::tr("btn-releases-add-btn"), Wt::XHTMLText);
		entry->bindWidget("add-btn", addBtn);
		addBtn->clicked().connect(std::bind([=]
		{
			releaseAdd.emit(releaseId);
		}));
	}

	std::cerr << "if-show-more = " << moreResults << "\n";

	_showMore->setHidden(!moreResults);
}

} // namespace UserInterface

