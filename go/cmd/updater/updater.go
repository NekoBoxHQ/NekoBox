package main

import (
	"context"
	"log"
	"os"
	"path/filepath"
	"runtime"
	"strings"

	"github.com/codeclysm/extract"
)

func Updater() {
	pre_cleanup := func() {
		if runtime.GOOS == "linux" {
			os.RemoveAll("./usr")
		}
		os.RemoveAll("./nekobox_update")
	}

	// find update package
	var updatePackagePath string
	if len(os.Args) == 2 && Exist(os.Args[1]) {
		updatePackagePath = os.Args[1]
	} else if Exist("./nekobox_update.zip") {
		updatePackagePath = "./nekobox_update.zip"
	} else if Exist("./nekobox_update.tar.gz") {
		updatePackagePath = "./nekobox_update.tar.gz"
	} else {
		log.Fatalln("no update")
	}
	log.Println("updating from", updatePackagePath)

	// extract update package
	if strings.HasSuffix(updatePackagePath, ".zip") {
		pre_cleanup()
		f, err := os.Open(updatePackagePath)
		if err != nil {
			log.Fatalln(err.Error())
		}
		err = extract.Zip(context.Background(), f, "./nekobox_update", nil)
		if err != nil {
			log.Fatalln(err.Error())
		}
		f.Close()
	} else if strings.HasSuffix(updatePackagePath, ".tar.gz") {
		pre_cleanup()
		f, err := os.Open(updatePackagePath)
		if err != nil {
			log.Fatalln(err.Error())
		}
		err = extract.Gz(context.Background(), f, "./nekobox_update", nil)
		if err != nil {
			log.Fatalln(err.Error())
		}
		f.Close()
	}

	// remove old file
	removeAll("./*.dll")
	removeAll("./*.dmp")

	// update move
	var updateSrc string
	if runtime.GOOS == "windows" {
		updateSrc = "./nekobox_update"
	} else {
		updateSrc = "./nekobox_update/nekobox"
	}
	err := Mv(updateSrc, "./")
	if err != nil {
		MessageBoxPlain("NekoGui Updater", "Update failed. Please close the running instance and run the updater again.\n\n"+err.Error())
		log.Fatalln(err.Error())
	}

	os.RemoveAll("./nekobox_update")
	os.RemoveAll("./nekobox_update.zip")
	os.RemoveAll("./nekobox_update.tar.gz")

	// nekoray -> nekobox
	os.Remove("./nekoray.exe")
	os.Remove("./nekoray.png")
	os.Remove("./nekoray_core.exe")
}

func Exist(path string) bool {
	_, err := os.Stat(path)
	return err == nil
}

func FindExist(paths []string) string {
	for _, path := range paths {
		if Exist(path) {
			return path
		}
	}
	return ""
}

func Mv(src, dst string) error {
	s, err := os.Stat(src)
	if err != nil {
		return err
	}
	if s.IsDir() {
		es, err := os.ReadDir(src)
		if err != nil {
			return err
		}
		for _, e := range es {
			err = Mv(filepath.Join(src, e.Name()), filepath.Join(dst, e.Name()))
			if err != nil {
				return err
			}
		}
	} else {
		err = os.MkdirAll(filepath.Dir(dst), 0755)
		if err != nil {
			return err
		}
		// Windows os.Rename fails if dst exists; remove it first
		if _, statErr := os.Stat(dst); statErr == nil {
			if removeErr := os.RemoveAll(dst); removeErr != nil {
				return removeErr
			}
		}
		err = os.Rename(src, dst)
		if err != nil {
			return err
		}
	}
	return nil
}

func removeAll(glob string) {
	files, _ := filepath.Glob(glob)
	for _, f := range files {
		os.Remove(f)
	}
}
