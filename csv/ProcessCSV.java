/* 
* 
*
*  A driver class for processing a CSV file.
*  This file instantiates a CSVReader from a CSV file, and performs some sorts and prints.
*
*
*  Erik Bertse
*
*/

package eb;

import java.io.*;
import java.util.*;



public class ProcessCSV {
	
	

	public static void main(String[] args) {

		//The input file
		String file = "Sortiment.csv";

		//A new CSVReader
		CSVReader csv = new CSVReader(file);

			/* Columns: 
			0 is ArtikelID
			1 is Namn
			2 is Namn2
			3 is Prisinklmoms
			4 is Volymiml
			5 is PrisPerLiter
			6 is Varugrupp
			7 is Typ
			8 is Stil
			9 is ursprung
			10 is ursprungslandnamn
			11 is Producent
			12 is Argang			
			13 is Alkoholhalt
			14 is SortimentText
			15 is Ekologisk
			16 is Etiskt
			17 is RavarorBeskrivning
		*/

		// XXX Count the number of times Sverige appears as ursprungsland.
		String findName = "Sverige";
		int findInColumn = 10;
		int numInColumn = csv.count(findInColumn, findName);
		System.out.println("    -- Antalet artiklar producerade i Sverige: " + numInColumn + "\n");

		// XXX Count the number of times Öl appears as Varugrupp.
		findName = "Öl";
		findInColumn = 6;

		numInColumn = csv.count(findInColumn, findName);
		System.out.println("    -- Antalet artiklar vars varugrupp är öl: " + numInColumn + "\n");

		// XXX Count the number of times Typ is empty.
		findInColumn = 7;
		numInColumn = csv.countEmpty(findInColumn);
		System.out.println("    -- Antalet tomma fält i Kolumn: \"" + csv.nameOf(findInColumn) + "\": " + numInColumn + "\n");


		// XXX Count the number of products in each Varugrupp.
		System.out.println("    -- Så här många varor finns i varje varugrupp:");
		findInColumn = 6;
		
		//Sort by varugrupp
		csv.sortByColumn(findInColumn);
		
		//Get the first varugrupp
		String temp = csv.get(0,findInColumn);
		int counter = 0;
		for (int i = 0; i < csv.size(); i++) {

			String varugrupp = csv.get(i,findInColumn);
			
			//If Varugrupp is hasn't changed, increment the counter.
			// Otherwise, print the counter and Varugrupp, and change the current Varugrupp
			if (varugrupp.equals(temp)) {
				counter++;
			} else {
				System.out.printf ("%-24s %4d %s \n", temp,  counter, "st");
				temp = varugrupp;
				counter = 1;
			}
			// Since the last Varugrupp never changes, at the last index, we print out
			// the current Varugrupp and counter.
			if (i == csv.size() - 1) {
				System.out.printf ("%-24s %4d %s \n", temp,  counter, "st");
			}
			
		}

		
		System.out.println("");

		// XXX Find most expensive product in PrisPerLiter
		System.out.println("    -- Dyraste artikeln i filen, i PrisPerLiter");
		findInColumn = 5;

		//Sort by prisPerLiter in decreasing order
		csv.sortByColumn(findInColumn, 0, csv.size(), 1);

		//Print relevant data for the top product
		System.out.println(" - ArtikelID: " + csv.get(0,0));
		System.out.println(" - Namn: " + csv.get(0,1));
		System.out.println(" - Prisinklmoms: " + csv.get(0,3));
		System.out.println(" - PrisPerLiter: " + csv.get(0,5));
		System.out.println(" - Varugrupp: " + csv.get(0,6));
		System.out.println(" - Ursprungsland: " + csv.get(0,10));

		System.out.println("");
		
		// XXX Find, for each varugrupp, products with low alcohol content.
		System.out.println("    -- Dessa artiklar har alkoholhalt <= 50% i vardera varugrupp:  \n");
		

		//Sort by first name, and then Varugrupp, since MergeSort is stable, this garantees
		//that after we sort by Varugrupp, the names are still sorted.
		csv.sortByColumn(1, 0, csv.size(), 0);
		csv.sortByColumn(6, 0, csv.size(), 0);

		temp = null;
		for (int i = 0; i < csv.size(); i++) {

			String varugrupp = csv.get(i,6);
			
			//If we found a new Varugrupp, print its name
			if (!varugrupp.equals(temp)) {
				System.out.println(" - Varugrupp: " + varugrupp);
				temp = varugrupp;
			}

			//Parse percentage as float
			String percentage = csv.get(i,13);
			percentage = percentage.replace("%", "");
			float f = Float.parseFloat(percentage);

			//If it's in the right range, print it.
			if ( 0 <= f && f <= 0.5) {
				System.out.printf("%-8s %-38s %-8s %1.2f%% \n", "- Namn: ", csv.get(i,1), " Halt: ", f);
			}
			
		}

		// XXX Find the average strength of Whiskey.
		System.out.println("");
		System.out.print("    -- Genomsnittliga alkoholhalten av Whisky: ");
		float avg = 0;
		for (int i = 0; i < csv.size(); i++) {

			String varugrupp = csv.get(i,6);
			
			//If varugrupp is whisky, we add its percentage to the variable avg.
			if (varugrupp.equals("Whisky")) {
				String percentage = csv.get(i,13);
				percentage = percentage.replace("%", "");
				avg += Float.parseFloat(percentage);
			}
		}
		//Divide by the number of entries with Varugrupp Whisky.
		avg = avg / csv.count(6, "Whisky");
		System.out.print(avg + " % \n \n");

		// XXX Find the strongest product for each country.
		System.out.println("    -- Starkaste varan per land: ");
		
		//Sort by Alkoholhalt in decreasing order.
		//Sort by country in increasing order.
		//The first product for each country is now its strongest product.
		csv.sortByColumn(13, 0, csv.size(), 1);
		csv.sortByColumn(10);
		temp = null;
		for (int i = 0; i < csv.size(); i++) {

			String land = csv.get(i,10);
			
			//If the current land changes, print data for the product on top.
			if (!land.equals(temp)) {
				String percentage = csv.get(i,13);
				System.out.printf("Land: %-22s  Namn: %-20s  Halt: %s \n", land, csv.get(i,1), percentage);
				temp = land;
			}
			
		}
		System.out.println("");

		
		
		// XXX Find the median strength beer.
		System.out.println("    -- Ölen vars styrka är median: ");
		csv.sortByColumn(13);
		csv.sortByColumn(6);
		numInColumn = csv.count(6, "Öl");

		temp = null;
		for (int i = 0; i < csv.size(); i++) {

			String varugrupp = csv.get(i,6);
			
			if (varugrupp.equals("Öl")) {
				
				String percentage = csv.get(i + numInColumn/2 ,13);
				System.out.printf("Land: %-22s  Namn: %-20s  Halt: %s \n", csv.get(i,10), csv.get(i,1), percentage);
				break;
			}
			
		}



	}


}
//EOF