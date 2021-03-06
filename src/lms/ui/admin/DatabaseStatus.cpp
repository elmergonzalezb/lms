/*
 * Copyright (C) 2019 Emeric Poupon
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

#include "DatabaseStatus.hpp"

#include <iomanip>

#include <Wt/Http/Response.h>
#include <Wt/WDateTime.h>
#include <Wt/WPushButton.h>
#include <Wt/WResource.h>

#include "utils/Service.hpp"
#include "LmsApplication.hpp"

namespace UserInterface {

static
std::string
durationToString(const Wt::WDateTime& begin, const Wt::WDateTime& end)
{
	const auto secs {std::chrono::duration_cast<std::chrono::seconds>(end.toTimePoint() - begin.toTimePoint()).count()};

	std::ostringstream oss;

	if (secs >= 3600)
		oss << secs/3600 << "h";
	if (secs >= 60)
		oss << std::setw(2) << std::setfill('0') << (secs % 3600) / 60 << "m";
	oss << std::setw(2) << std::setfill('0') << (secs % 60) << "s";

	return oss.str();
}


class ReportResource : public Wt::WResource
{
	public:

		ReportResource(const Scanner::ScanStats& stats)
		: _stats {stats}
		{
			suggestFileName("report.txt");
		}

		~ReportResource()
		{
			beingDeleted();
		}

		void handleRequest(const Wt::Http::Request&, Wt::Http::Response& response)
		{
			response.out() << Wt::WString::tr("Lms.Admin.Database.Status.errors-header").arg(_stats.errors.size()).toUTF8() << std::endl;

			for (const auto& error : _stats.errors)
			{
				response.out() << error.file.string() << " - " << errorTypeToWString(error.error).toUTF8();
				if (!error.systemError.empty())
					response.out() << ": " << error.systemError;
				response.out() << std::endl;
			}

			response.out() << std::endl;

			response.out() << Wt::WString::tr("Lms.Admin.Database.Status.duplicates-header").arg(_stats.duplicates.size()).toUTF8() << std::endl;

			for (const auto& duplicate : _stats.duplicates)
				response.out() << duplicate.file.string() << " - " << duplicateReasonToWString(duplicate.reason).toUTF8() << std::endl;
		}

	private:

		static Wt::WString errorTypeToWString(Scanner::ScanErrorType error)
		{
			switch (error)
			{
				case Scanner::ScanErrorType::CannotReadFile: return Wt::WString::tr("Lms.Admin.Database.Status.cannot-read-file");
				case Scanner::ScanErrorType::CannotParseFile: return Wt::WString::tr("Lms.Admin.Database.Status.cannot-parse-file");
				case Scanner::ScanErrorType::NoAudioTrack: return Wt::WString::tr("Lms.Admin.Database.Status.no-audio-track");
				case Scanner::ScanErrorType::BadDuration: return Wt::WString::tr("Lms.Admin.Database.Status.bad-duration");
			}
			return "?";
		}

		static Wt::WString duplicateReasonToWString(Scanner::DuplicateReason reason)
		{
			switch (reason)
			{
				case Scanner::DuplicateReason::SameHash: return Wt::WString::tr("Lms.Admin.Database.Status.same-hash");
				case Scanner::DuplicateReason::SameMBID: return Wt::WString::tr("Lms.Admin.Database.Status.same-mbid");
			}
			return "?";
		}

		Scanner::ScanStats _stats;
};


DatabaseStatus::DatabaseStatus()
: WTemplate {Wt::WString::tr("Lms.Admin.Database.Status.template")}
{
	addFunction("tr", &Wt::WTemplate::Functions::tr);

	using namespace Scanner;

	auto onDbEvent = [&]() { refreshContents(); };

	LmsApp->getEvents().dbScanned.connect(this, onDbEvent);
	LmsApp->getEvents().dbScanInProgress.connect(this, onDbEvent);
	LmsApp->getEvents().dbScanScheduled.connect(this, onDbEvent);

	refreshContents();
}

void
DatabaseStatus::refreshContents()
{
	using namespace Scanner;

	Wt::WPushButton* reportBtn {bindNew<Wt::WPushButton>("btn-report", Wt::WString::tr("Lms.Admin.Database.Status.get-report"))};

	const IMediaScanner::Status status {ServiceProvider<IMediaScanner>::get()->getStatus()};
	if (status.lastCompleteScanStats)
	{
		bindString("last-scan", Wt::WString::tr("Lms.Admin.Database.Status.last-scan-status")
				.arg(status.lastCompleteScanStats->nbFiles())
				.arg(durationToString(status.lastCompleteScanStats->startTime, status.lastCompleteScanStats->stopTime))
				.arg(status.lastCompleteScanStats->stopTime.toString())
				.arg(status.lastCompleteScanStats->errors.size())
				.arg(status.lastCompleteScanStats->duplicates.size())
			  );

		Wt::WLink link {std::make_shared<ReportResource>(*status.lastCompleteScanStats)};
		link.setTarget(Wt::LinkTarget::NewWindow);
		reportBtn->setLink(link);
	}
	else
	{
		bindString("last-scan", Wt::WString::tr("Lms.Admin.Database.Status.last-scan-not-available"));
		reportBtn->setEnabled(false);
	}


	switch (status.currentState)
	{
		case IMediaScanner::State::NotScheduled:
			bindString("status", Wt::WString::tr("Lms.Admin.Database.Status.status-not-scheduled"));
			break;
		case IMediaScanner::State::Scheduled:
			bindString("status", Wt::WString::tr("Lms.Admin.Database.Status.status-scheduled")
					.arg(status.nextScheduledScan.toString()));
			break;
		case IMediaScanner::State::InProgress:
			{
				std::ostringstream oss;
				bindString("status", Wt::WString::tr("Lms.Admin.Database.Status.status-in-progress")
						.arg(status.inProgressScanStats->processedFiles)
						.arg(status.inProgressScanStats->filesToScan)
						.arg(status.inProgressScanStats->progress()));
			}
			break;
	}

}

} // namespace UserInterface

