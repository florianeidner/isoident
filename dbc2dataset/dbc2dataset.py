# encoding: utf-8

import codecs

def file_len(fname):
    with open(fname) as f:
        for i, l in enumerate(f):
            pass
    return i + 1

def main():

	print "Please create a .dbc file in the running dir of this script called dataset.dbc"

	raw_input("Hit ENTER to start")

	fname = "dataset.dbc"

	dbc_file  = codecs.open(fname, "r","iso-8859-1")

	csv_file = codecs.open("signals.CSV","w","utf-8")

	dbc_file_lines = dbc_file.readlines()

	for i in range(0, len(dbc_file_lines)-1):
		
		dbcline = dbc_file_lines[i]

		if (dbcline[:3] == "BO_"):
			pgnid = dbcline[4:14]
			pgn = (int(pgnid) >> 8) & 0x00FFFF
			print "Found Line with PGN:"+str(pgn)
			
			if pgn == 0:
				continue

			i=i+1

			spnline = dbc_file_lines[i]

			while (spnline[:4] == " SG_"):
				print "Found Signal"
				#print spnline

				spn_details1 = spnline.split(":")[0].split(" ")
				spn_details2 = spnline.split(":")[1].split(" ")
				
				spn_name = spn_details1[2]
				print " -> Name:     "+spn_name
				
				spn_startbit = spn_details2[1].split("|")[0]
				print " -> Startbit: "+spn_startbit
				spn_len = spn_details2[1].split("|")[1].split("@")[0]
				print " -> Length:   "+spn_len


				spn_factor = spn_details2[2].split(",")[0][1:]
				print " -> Factor:   "+spn_factor
				spn_offset = spn_details2[2].split(",")[1][:-1]
				print " -> Offset:   "+spn_offset

				spn_range_lower = spn_details2[3].split("|")[0][1:]
				print " -> Min:   "+spn_range_lower
				spn_range_higher = spn_details2[3].split("|")[1][:-1]
				print " -> Max:   "+spn_range_higher

				spn_unit = spn_details2[4][1:-1]
				if spn_unit == "":
					spn_unit = "-"
				print " -> Unit:   "+spn_unit


				for j in range(0, len(dbc_file_lines)-1):


					if (dbc_file_lines[j][:14]=='BA_ "SPN" SG_ '):


						if (dbc_file_lines[j][14:].split(" ")[1]== spn_name):

							spn_id = dbc_file_lines[j][14:].split(" ")[2][:-3]

							print " -> SPN:    "+spn_id
							
							csv_file.write(str(pgn)+";"+spn_id+";"+spn_name+";"+spn_startbit+";"+spn_len+";"+spn_unit+";"+spn_offset+";"+spn_factor+";"+spn_range_lower+";"+spn_range_higher+"\n")

							break

				print ""
				i=i+1

				spnline = dbc_file_lines[i]

	dbc_file.close()
	csv_file.close()



main()