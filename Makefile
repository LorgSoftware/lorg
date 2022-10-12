# Lorg

include config.mk

RELEASE_BIN = lorg
BUILD_RELEASE_DIR = build
DEBUG_BIN = lorg-debug
BUILD_DEBUG_DIR = build-debug
MAN_FILE = lorg.1

INSTALL_BIN_DIR = ${DESTDIR}${PREFIX}/bin
INSTALL_MAN_DIR = ${DESTDIR}${MANPREFIX}/man1

release:
	mkdir -p ${BUILD_RELEASE_DIR}
	cd ${BUILD_RELEASE_DIR} && cmake -DCMAKE_BUILD_TYPE=Release .. && cmake --build .
	mv ${BUILD_RELEASE_DIR}/${RELEASE_BIN} ${RELEASE_BIN}

debug:
	mkdir -p ${BUILD_DEBUG_DIR}
	cd ${BUILD_DEBUG_DIR} && cmake -DCMAKE_BUILD_TYPE=Debug .. && cmake --build .
	mv ${BUILD_DEBUG_DIR}/${DEBUG_BIN} ${DEBUG_BIN}

clean:
	rm -rf ${BUILD_RELEASE_DIR} ${RELEASE_BIN}
	rm -rf ${BUILD_DEBUG_DIR} ${DEBUG_BIN}

install:
	mkdir -p ${INSTALL_BIN_DIR}
	cp -f ${RELEASE_BIN} ${INSTALL_BIN_DIR}
	chmod 755 ${INSTALL_BIN_DIR}/${RELEASE_BIN}
	mkdir -p ${INSTALL_BIN_DIR}
	sed "s/VERSION/${VERSION}/g" < ${MAN_FILE} > ${INSTALL_MAN_DIR}/${MAN_FILE}
	chmod 644 ${INSTALL_MAN_DIR}/${MAN_FILE}

uninstall:
	rm -f ${INSTALL_BIN_DIR}/${RELEASE_BIN}
	rm -f ${INSTALL_MAN_DIR}/${MAN_FILE}

.PHONY: release debug clean install uninstall
