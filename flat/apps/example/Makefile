default:
	cd makeprocs; make
	cd ostests1; make
	cd ostests2; make

clean:
	cd makeprocs; make clean
	cd ostests1; make clean
	cd ostests2; make clean

run:
	cd ../../bin; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj 3; ee469_fixterminal
