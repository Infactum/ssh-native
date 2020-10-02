from conans import ConanFile


class Pkg(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"
    requires = (
        "boost/1.70.0",
        "libssh/0.9.4@infactum/stable",
    )

    def configure(self):
        self.options["boost"].shared = False
        self.options["libssh"].shared = False

    def requirements(self):
        if self.settings.os == "Linux":
            self.requires("libuuid/1.0.3")
