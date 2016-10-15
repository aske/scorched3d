{ stdenv, fetchFromGitHub, mesa, glew, pkgconfig, openalSoft, freealut, wxGTK, libogg
, freetype, libvorbis, fftwSinglePrec, SDL, SDL_net, expat, libjpeg, libpng
, perl, autoconf, automake }:

stdenv.mkDerivation rec {
  version = "44";
  name = "scorched3d-${version}";
  src = ./scorched;

  buildInputs =
    [ mesa glew openalSoft freealut wxGTK libogg freetype libvorbis
      SDL SDL_net expat libjpeg libpng fftwSinglePrec
    ];

  nativeBuildInputs = [ pkgconfig perl autoconf automake ];

  sourceRoot = "scorched";

  configureFlags = [ "--with-fftw=${fftwSinglePrec.dev}" ];

  NIX_LDFLAGS = [ "-lopenal" ];

  preConfigure = "sh autogen.sh";

  meta = with stdenv.lib; {
    homepage = http://scorched3d.co.uk/;
    description = "3D Clone of the classic Scorched Earth";
    license = licenses.gpl2Plus;
    platforms = platforms.linux;
  };
}
