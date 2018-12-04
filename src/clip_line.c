#define outcodes(outcode, x, y, wxmin, wymin, wxmax, wymax) { \
    outcode[0] = (y) > (wymax); \
    outcode[1] = (y) < (wymin); \
    outcode[2] = (x) > (wxmax); \
    outcode[3] = (x) < (wxmin); \
}

#define swap(type, outcode1, outcode2, x1, y1, x2, y2) { \
	type tf = (x1); \
	int i; \
	\
    (x1) = (x2); \
    (x2) = tf; \
    tf = (y1); \
    (y1) = (y2); \
    (y2) = tf; \
	\
    for(i = 0; i < 4; i++) \
    { \
    int ti = outcode1[i]; \
		\
        outcode1[i] = outcode2[i]; \
        outcode2[i] = ti; \
    } \
}

int clip_line(float *x1, float *y1, float *x2, float *y2,
	float wxmin, float wymin, float wxmax, float wymax)
{
char outcode1[4], outcode2[4];
int i, accept = 0;
	
	outcodes(outcode1, *x1, *y1, wxmin, wymin, wxmax, wymax);
	outcodes(outcode2, *x2, *y2, wxmin, wymin, wxmax, wymax);
	while(!accept) {
		for(i = 0; i < 4; i++) {
			if(outcode1[i] && outcode2[i])
				return 0;
		}
		
		for(accept = 1, i = 0; accept && i < 4; i++) 
			accept = !(outcode1[i] || outcode2[i]);
		
		if(!accept) {
			if(!(outcode1[0] || outcode1[1] || outcode1[2] || outcode1[3]))
				swap(float, outcode1, outcode2, *x1, *y1, *x2, *y2);
		 
			if(outcode1[0]) {
				*x1 += (*x2 - *x1) * (wymax - *y1) / (*y2 - *y1);
				*y1 = wymax;
			}
			else if(outcode1[1]) {
				*x1 += (*x2 - *x1) * (wymin - *y1) / (*y2 - *y1);
				*y1 = wymin;
			}
			else if(outcode1[2]) {
				*y1 += (*y2 - *y1) * (wxmax - *x1) / (*x2 - *x1);
				*x1 = wxmax;
			}
			else {
				*y1 += (*y2 - *y1) * (wxmin - *x1) / (*x2 - *x1);
				*x1 = wxmin;
			}
			
			outcodes(outcode1, *x1, *y1, wxmin, wymin, wxmax, wymax);
		}
	}
	
	return 1;
}

int clip_line2(int *x1, int *y1, int *x2, int *y2,
	int wxmin, int wymin, int wxmax, int wymax)
{
char outcode1[4], outcode2[4];
int i, accept = 0;
	
	outcodes(outcode1, *x1, *y1, wxmin, wymin, wxmax, wymax);
	outcodes(outcode2, *x2, *y2, wxmin, wymin, wxmax, wymax);
	while(!accept) {
		for(i = 0; i < 4; i++) {
			if(outcode1[i] && outcode2[i])
				return 0;
		}
		
		for(accept = 1, i = 0; accept && i < 4; i++) 
			accept = !(outcode1[i] || outcode2[i]);
		
		if(!accept) {
			if(!(outcode1[0] || outcode1[1] || outcode1[2] || outcode1[3]))
				swap(int, outcode1, outcode2, *x1, *y1, *x2, *y2);
		 
			if(outcode1[0]) {
				*x1 += (int)((float)(*x2 - *x1) * (wymax - *y1) / (*y2 - *y1));
				*y1 = wymax;
			}
			else if(outcode1[1]) {
				*x1 += (int)((float)(*x2 - *x1) * (wymin - *y1) / (*y2 - *y1));
				*y1 = wymin;
			}
			else if (outcode1[2]) {
				*y1 += (int)((float)(*y2 - *y1) * (wxmax - *x1) / (*x2 - *x1));
				*x1 = wxmax;
			}
			else {
				*y1 += (int)((float)(*y2 - *y1) * (wxmin-*x1) / (*x2 - *x1));
				*x1 = wxmin;
			}
			
			outcodes(outcode1, *x1, *y1, wxmin, wymin, wxmax, wymax);
		}
	}
	
	return 1;
}
