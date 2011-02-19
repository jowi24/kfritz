#!/bin/sh
grep -ir I18N_NOOP libfritz++/*.cpp | sed -e 's/.*I18N_NOOP(\([^)]*\).*/i18n(\1)/' | sort | uniq > LibFritzI18N.cpp
