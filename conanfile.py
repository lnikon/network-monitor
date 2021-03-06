from conans import ConanFile

class ConanPackage(ConanFile):
    name = 'network-monitor'
    version = "0.1.0"

    generators = 'cmake_find_package'

    requires = [
        ('boost/1.74.0'),
        ('openssl/1.1.1i'),
        ('libcurl/7.74.0'),
        ('nlohmann_json/3.9.1'),
        ('spdlog/1.8.5')
    ]

    default_options = (
        'boost:shared=False',
    )
