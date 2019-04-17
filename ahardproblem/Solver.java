/** 
 *  A hard problem.  
 *
 *  We are interested in the cheapest price possible subject to a given restriction.
 *  To find our restriction (length of the wall), we employ the GrahamScan algorithm
 *  which returns the convex hull. We calculate the length of this hull.
 *
 *  The issue of finding the cheapest possible price is an optimization problem,
 *  very similar to the classical Knapsack problem. (See Wikipedia).
 *  
 *  We use a dymanic programming technique to store, in a matrix m, the minimum prices 
 *  required for each length j <= wallLength, using i walls. 
 *  We have n walls so our required minimum price is m[n][wallLength]. 
 * 
 */

import java.util.List;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.Collections;
import java.util.Stack;

public class Solver {

    public Solver() {}
     
    //  Returns the smallest sum(p), such that sum(m) > limit.
    public int minimumKnapSack(int[] meters, int[] prices, double limit) {
        
        int intlimit = (int) (Math.ceil(limit));
        int [][] m = new int[meters.length][intlimit+1];
        
        for (int j = 0; j <= intlimit; j++) {
            // Here the subtraction avoids overflows.
            // While this is risky and bad practice, it satisfied original problem constraints.
            // Imagine the bugs...
            m[0][j] = Integer.MAX_VALUE - 50000000;
        }

        int n = meters.length - 1;

        for (int i = 1; i <= n; i++) {
            for (int j = 0; j <= intlimit; j++) {
                if (meters[i] >= j) {
                    m[i][j] = Math.min(m[i-1][j], prices[i]);
                } else {
                    m[i][j] = Math.min(m[i-1][j], m[i-1][j - meters[i]] + prices[i]);
                }
            }
        }
        
        return m[n][intlimit];
    }

    // return Length of the circumference of the convex hull
    public double grahamScan(List<Point> pointlist) {

        int minIdx = 0;
        for (int i= 0; i<pointlist.size(); i++) {
            Point p = pointlist.get(i);
            Point pmin = pointlist.get(minIdx);
            if (p.getY() < pmin.getY() ) {
                minIdx = i;
            } else if (p.getY() == pmin.getY() && p.getX() < pmin.getX() ) {
                minIdx = i;
            }
        }

        Collections.swap(pointlist, minIdx, 0);
        
        Point firstpoint = pointlist.get(0);
        Collections.sort(pointlist.subList(1,pointlist.size()), new Comparator<Point>() {
            public int compare(Point p1, Point p2) {
                int c = orient(firstpoint, p2, p1);
                if (c!=0) {
                    return c;
                } else {
                    return (lenSqr(p1,firstpoint) < lenSqr(p2,firstpoint)) ? -1 : 1;
                }

            }
        });
        
        Stack<Point> pointstack = new Stack<Point>();
        pointstack.push(pointlist.get(0));
        pointstack.push(pointlist.get(1));

        for (Point p : pointlist.subList(2, pointlist.size())) {
            while (pointstack.size() >= 2 && orient(pointstack.get(pointstack.size()-2), pointstack.peek(), p) <= 0) {
               pointstack.pop();
            }
            pointstack.push(p);
        }

        return getTotalLength(new ArrayList<Point>(pointstack));
    }
 
    //Helper function that returns the orientation of three points p0,p1,p2
    private int orient(Point p0, Point p1, Point p2) {
        return (p1.getX() - p0.getX())*(p2.getY() - p0.getY()) - 
               (p1.getY() - p0.getY())*(p2.getX() - p0.getX());
    }

    //Helper function that computes the length of the convex hull.
    private double getTotalLength(List<Point> wallpoints) {
        double ret = 0;
        int len = wallpoints.size();
        for (int i = 0; i<len; i++) {
            ret += Math.sqrt(lenSqr(wallpoints.get(i), wallpoints.get((i + 1) % len)));
        }
        return ret;

    }


    // Helper function that computes the length squared b between to nodes.
    private double lenSqr(Point p1, Point p2) {
        int dx = (p1.getX() - p2.getX());
        int dy = (p1.getY() - p2.getY());
        return dx*dx + dy*dy;
    }


}