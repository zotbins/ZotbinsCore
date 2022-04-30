# Pre-compilation script for platformio.ini
# Edit .env if you wish to change the values

from dataclasses import dataclass
import os
Import("env")

try:
    from dotenv import load_dotenv
except ImportError:
    env.Execute("$PYTHONEXE -m pip install python-dotenv")


CONFIG_HEADER = "config.hpp"
CONFIG_HEADER_CONTENTS = """
// AUTO GENERATED FILE, DO NOT EDIT
#ifndef CONFIG_HPP
#define CONFIG_HPP

namespace Zotbins
{{
    namespace Config
    {{
        const char *WIFI_SSID = \"{WIFI_SSID}\";
        const char *WIFI_PWD = \"{WIFI_PWD}\";
    }}
}}
#endif
"""


@dataclass
class Config:
    """Class for environment variables"""
    WIFI_SSID: str = ""
    WIFI_PWD: str = ""


def loadConfig() -> Config:
    # Return default if .env does not exist
    if not os.path.exists(".env"):
        return Config()

    # Load enviromental variables
    load_dotenv()
    WIFI_SSID = os.environ.get("ZBIN_CORE_WIFI_SSID")
    WIFI_PWD = os.environ.get("ZBIN_CORE_WIFI_PWD")

    return Config(WIFI_SSID, WIFI_PWD)


def createConfigHeader(config: Config):
    # Create header file
    if os.path.exists("include"):
        CONFIG_HEADER_PATH = "include" + os.sep + CONFIG_HEADER

        with open(CONFIG_HEADER_PATH, "w+") as file:
            file.write(CONFIG_HEADER_CONTENTS.format(
                WIFI_SSID=config.WIFI_SSID,
                WIFI_PWD=config.WIFI_PWD,
            ))
    else:
        raise FileNotFoundError("include/ directory not found")


# Load config and create config header file
config = loadConfig()
createConfigHeader(config)
