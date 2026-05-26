function Mail( whichName, domain )
{
	var base = "mailto:";
	var dom;

	if (domain == null || domain == "") {
		dom = "otakuworld.com";
		if (whichName == "Jenny") {
			base += "uni";
		} else if (whichName == "Dov") {
			base += "submitkiss";
		} else if (whichName == "SubmitKiSS") {
			base += "submitkiss";
		} else if (whichName == "Steve") {
			base += "stephenp";
		} else if (whichName == "Mascot") {
			base += "otakumascot";
		} else {
			base += "owmaster";
		}
	} else {
		base += whichName;
		dom = domain;
	}


	location.href = base + "@" + dom;
}
