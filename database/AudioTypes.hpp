#ifndef _AUDIO_TYPES_HPP_
#define _AUDIO_TYPES_HPP_

#include <string>
#include <vector>

#include <boost/filesystem.hpp>

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/WtSqlTraits>

#include <Wt/WDateTime>

class Track;
class Release;
class Artist;

class Artist
{
	public:

		typedef Wt::Dbo::ptr<Artist> pointer;

		Artist() {}
		Artist(const std::string& p_name);

		// Accessors
		static pointer getByName(Wt::Dbo::Session& session, const std::string& name);
		static pointer getNone(Wt::Dbo::Session& session);

		// Create
		static pointer create(Wt::Dbo::Session& session, const std::string& name);

		bool	isNone(void) const;

		template<class Action>
			void persist(Action& a)
			{
				Wt::Dbo::field(a, _name, "name");

				Wt::Dbo::hasMany(a, _tracks, Wt::Dbo::ManyToOne, "artist");
			}


	private:
		std::string _name;

		Wt::Dbo::collection< Wt::Dbo::ptr<Track> > _tracks;	// Tracks of this artist
};


// Album release
class Release
{
	public:

		typedef Wt::Dbo::ptr<Release> pointer;

		Release() {}
		Release(const std::string& name);

		// Accessors
		static pointer getByName(Wt::Dbo::Session& session, const std::string& name);
		static pointer getNone(Wt::Dbo::Session& session);

		// Create
		static pointer create(Wt::Dbo::Session& session, const std::string& name);

		std::string	getName() const;
		bool		isNone(void) const;

		template<class Action>
			void persist(Action& a)
			{
				Wt::Dbo::field(a, _name, "name");

				Wt::Dbo::hasMany(a, _tracks, Wt::Dbo::ManyToOne, "release");
			}

	private:
		std::string _name;

		Wt::Dbo::collection< Wt::Dbo::ptr<Track> > _tracks;	// Tracks in the release
};


class Genre
{
	public:

		typedef Wt::Dbo::ptr<Genre> pointer;

		Genre();
		Genre(const std::string& name);

		// Find utility
		static pointer getByName(Wt::Dbo::Session& session, const std::string& name);
		static pointer getNone(Wt::Dbo::Session& session);

		// Create utility
		static pointer create(Wt::Dbo::Session& session, const std::string& name);

		// Accessors
		const std::string& getName(void) const { return _name; }
		bool isNone(void) const;

		template<class Action>
			void persist(Action& a)
			{
				Wt::Dbo::field(a, _name,	"name");
				Wt::Dbo::hasMany(a, _tracks, Wt::Dbo::ManyToMany, "track_genre", "", Wt::Dbo::OnDeleteCascade);
			}

	private:
		std::string	_name;

		Wt::Dbo::collection< Wt::Dbo::ptr<Track> > _tracks;
};

class Track
{
	public:

		typedef Wt::Dbo::ptr<Track> pointer;

		Track() {}
		Track(const boost::filesystem::path& p, Artist::pointer artist, Release::pointer release);

		// Find utilities
		static pointer getByPath(Wt::Dbo::Session& session, const boost::filesystem::path& p);
		static Wt::Dbo::collection< pointer > getAll(Wt::Dbo::Session& session);

		// Create utility
		static pointer	create(Wt::Dbo::Session& session, const boost::filesystem::path& p, Artist::pointer artist, Release::pointer release);

		// Accessors
		void setTrackNumber(int num)					{ _trackNumber = num; }
		void setDiscNumber(int num)					{ _discNumber = num; }
		void setName(const std::string& name)				{ _name = name; }
		void setDuration(boost::posix_time::time_duration duration)	{ _duration = duration; }
		void setLastWriteTime(boost::posix_time::ptime time)		{ _fileLastWrite = time; }
		void setChecksum(const std::vector<unsigned char>& checksum)	{ _fileChecksum = checksum; }
		void setCreationTime(const boost::posix_time::ptime& time)	{ _creationTime = time; }
		void setGenres(const std::string& genreList)			{ _genreList = genreList; }
		void setGenres(std::vector<Genre::pointer> genres);
		void setArtist(Artist::pointer artist)				{ _artist = artist; }
		void setRelease(Release::pointer release)			{ _release = release; }

		std::string 		getName(void) const			{ return _name; }
		const std::string&	getPath(void) const			{ return _filePath; }
		boost::posix_time::time_duration	getDuration(void) const	{ return _duration; }
		boost::posix_time::ptime	getLastWriteTime(void) const	{ return _fileLastWrite; }
		const std::vector<unsigned char>& getChecksum(void) const	{ return _fileChecksum; }
		Artist::pointer getArtist(void)	const				{ return _artist; }
		Release::pointer getRelease(void) const				{ return _release; }
		bool hasGenre(Genre::pointer genre) const			{ return _genres.count(genre); }
		std::vector< Genre::pointer > getGenres(void) const;

		template<class Action>
			void persist(Action& a)
			{
				Wt::Dbo::field(a, _trackNumber,		"track_number");
				Wt::Dbo::field(a, _discNumber,		"disc_number");
				Wt::Dbo::field(a, _name,		"name");
				Wt::Dbo::field(a, _duration,		"duration");
				Wt::Dbo::field(a, _creationTime,	"creation_time");
				Wt::Dbo::field(a, _genreList,		"genre_list");
				Wt::Dbo::field(a, _filePath,		"path");
				Wt::Dbo::field(a, _fileLastWrite,	"last_write");
				Wt::Dbo::field(a, _fileChecksum,	"checksum");
				Wt::Dbo::hasMany(a, _genres, Wt::Dbo::ManyToMany, "track_genre", "", Wt::Dbo::OnDeleteCascade);
				Wt::Dbo::belongsTo(a, _release, "release", Wt::Dbo::OnDeleteCascade);
				Wt::Dbo::belongsTo(a, _artist, "artist", Wt::Dbo::OnDeleteCascade);
			}

	private:
		int					_trackNumber;
		int					_discNumber;
		std::string				_name;
		boost::posix_time::time_duration	_duration;
		boost::posix_time::ptime		_creationTime;
		std::string				_genreList;
		std::string				_filePath;
		std::vector<unsigned char>		_fileChecksum;
		boost::posix_time::ptime		_fileLastWrite;

		Artist::pointer		_artist;		// Associated Artist
		Release::pointer	_release;		// Associated Release
		Wt::Dbo::collection< Genre::pointer > _genres;	// Tracks in the release
};



#endif
