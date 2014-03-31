#ifndef AUDIO_WIDGET_HPP
#define AUDIO_WIDGET_HPP

#include <string>

#include <Wt/WLink>
#include <Wt/WImage>
#include <Wt/WContainerWidget>
#include <Wt/WMemoryResource>

#include "audio/AudioMediaPlayerWidget.hpp"
#include "audio/AudioDatabaseWidget.hpp"

class AudioWidget : public Wt::WContainerWidget
{

	public:

		AudioWidget(Wt::WContainerWidget* parent = 0);

		void search(const std::string& searchText);

	private:

		void playTrack(boost::filesystem::path p);

		void handleTrackEnded(void);

		AudioDatabaseWidget*	_audioDbWidget;

		AudioMediaPlayerWidget*	_mediaPlayer;

		// Image
		Wt::WMemoryResource	*_imgResource;
		Wt::WLink		_imgLink;
		Wt::WImage		*_img;



};

#endif
