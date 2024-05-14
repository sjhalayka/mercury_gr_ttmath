#include "main.h"


int main(int argc, char** argv)
{
	cout << setprecision(20) << endl;

	//double f = 12434.555567798;

	//	ostringstream oss;
	//	oss << std::fixed << setprecision(7);
	//	oss << f;

	//	istringstream iss(oss.str());

	//	double x = 0;

	//	iss >> x;

	//	cout << x;

	//	exit(0);



	while (1)
	{
		idle_func();
	}

	return 0;

	glutInit(&argc, argv);
	init_opengl(win_x, win_y);
	glutReshapeFunc(reshape_func);
	glutIdleFunc(idle_func);
	glutDisplayFunc(display_func);
	glutKeyboardFunc(keyboard_func);
	glutMouseFunc(mouse_func);
	glutMotionFunc(motion_func);
	glutPassiveMotionFunc(passive_motion_func);
	//glutIgnoreKeyRepeat(1);
	glutMainLoop();
	glutDestroyWindow(win_id);

	return 0;
}

custom_math::vector_3 grav_acceleration(const custom_math::vector_3& pos, const custom_math::vector_3& vel, const long double G)
{
	custom_math::vector_3 grav_dir = sun_pos - pos;

	const double distance = grav_dir.length();
	grav_dir.normalize();

	custom_math::vector_3 accel = grav_dir * G * sun_mass / (distance * distance);

	return accel;
}



// Stolen from Stack Exchange
double precision(double f, int places)
{
	long double n = std::pow(10.0f, places);
	return std::round(f * n) / n;
}


double precision2(double f, int places)
{
	return f;

	static ostringstream oss;
	oss.clear();
	oss << std::fixed << setprecision(places) << f;

	static istringstream iss;
	iss.clear();
	iss.str(oss.str());

	double x = 0;
	iss >> x;

	return x;
}


void proceed_Euler(custom_math::vector_3& pos, custom_math::vector_3& vel, const long double G, const long double dt)
{
	const custom_math::vector_3 grav_dir = sun_pos - pos;
	const double distance = grav_dir.length();
	const double Rs = 2 * grav_constant * sun_mass / (speed_of_light * speed_of_light);

	const double alpha = 2.0 - sqrt(1 - (vel.length() * vel.length()) / (speed_of_light * speed_of_light));

	double beta = sqrt(1.0 - Rs / distance);

	//beta = precision2(beta, 3);

	beta = static_cast<float>(beta);

	//cout << precision2(beta, 9) << " " << beta << endl;



	custom_math::vector_3 accel = grav_acceleration(pos, vel, G);// *(1.0 / beta);

	vel += accel * dt * alpha;
	pos += vel * dt * beta;
}


long double total = 0;
long unsigned int frame_count = 0;

void idle_func(void)
{
	frame_count++;

	const long double dt = 0.01;// 5e-6 * (speed_of_light / mercury_vel.length());

	custom_math::vector_3 last_pos = mercury_pos;

	proceed_Euler(mercury_pos, mercury_vel, grav_constant, dt);


	/// this doesn't work all of the time... change it by keeping track of previous two positions


	if (decreasing)
	{
		if (mercury_pos.length() > last_pos.length())
		{
			// hit perihelion
			cout << "hit perihelion" << endl;
			decreasing = false;
			return;
		}
	}
	else
	{
		if (mercury_pos.length() < last_pos.length())
		{
			// hit aphelion
			cout << "hit aphelion" << endl;

			orbit_count++;

			custom_math::vector_3 current_dir = last_pos;
			current_dir.normalize();

			const long double d = current_dir.dot(previous_dir);

			const long double angle = acos(d);
			previous_dir = current_dir;

			custom_math::vector_3 temp_pos = last_pos;
			temp_pos.rotate_z(-total);

			if (temp_pos.x < 0)
				total += angle;
			else
				total -= angle;

			const long double avg = total / orbit_count;

			static const long double num_orbits_per_earth_century = 365.0 / 88.0 * 100;
			static const long double to_arcseconds = 1.0 / (pi / (180.0 * 3600.0));

			cout << "orbit " << orbit_count << endl;
			cout << "dot   " << d << endl;
			cout << "total " << total * num_orbits_per_earth_century * to_arcseconds << endl;
			cout << "angle " << angle * num_orbits_per_earth_century * to_arcseconds << endl;
			cout << "delta " << delta * num_orbits_per_earth_century * to_arcseconds << endl;
			cout << "avg   " << avg   * num_orbits_per_earth_century * to_arcseconds << endl;

			cout << endl;

			positions.clear();

			decreasing = true;
		}
	}


	// Commented out due to performance reason
	// positions.push_back(mercury_pos);

	// Commented out due to performance reason
	//if(frame_count % 60000000 == 0)
	//	glutPostRedisplay();
}

void init_opengl(const int& width, const int& height)
{
	win_x = width;
	win_y = height;

	if (win_x < 1)
		win_x = 1;

	if (win_y < 1)
		win_y = 1;

	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(win_x, win_y);
	win_id = glutCreateWindow("orbit");

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glClearColor(background_colour.x, background_colour.y, background_colour.z, 1);
	glClearDepth(1.0f);

	main_camera.Set(0, 0, camera_w, camera_fov, win_x, win_y, camera_near, camera_far);
}

void reshape_func(int width, int height)
{
	win_x = width;
	win_y = height;

	if (win_x < 1)
		win_x = 1;

	if (win_y < 1)
		win_y = 1;

	glutSetWindow(win_id);
	glutReshapeWindow(win_x, win_y);
	glViewport(0, 0, win_x, win_y);

	main_camera.Set(main_camera.u, main_camera.v, main_camera.w, main_camera.fov, win_x, win_y, camera_near, camera_far);
}

// Text drawing code originally from "GLUT Tutorial -- Bitmap Fonts and Orthogonal Projections" by A R Fernandes
void render_string(int x, const int y, void* font, const string& text)
{
	for (size_t i = 0; i < text.length(); i++)
	{
		glRasterPos2i(x, y);
		glutBitmapCharacter(font, text[i]);
		x += glutBitmapWidth(font, text[i]) + 1;
	}
}
// End text drawing code.

void draw_objects(void)
{
	glDisable(GL_LIGHTING);

	glPushMatrix();


	glPointSize(1.0);
	glLineWidth(1.0);


	glBegin(GL_POINTS);
	glVertex3f(sun_pos.x, sun_pos.y, sun_pos.z);

	glColor3f(1.0, 1.0, 1.0);

	for (size_t i = 0; i < positions.size(); i++)
		glVertex3d(positions[i].x, positions[i].y, positions[i].z);

	glEnd();





	glLineWidth(1.0f);


	// If we do draw the axis at all, make sure not to draw its outline.
	if (true == draw_axis)
	{
		glBegin(GL_LINES);

		glColor3f(1, 0, 0);
		glVertex3f(0, 0, 0);
		glVertex3f(1, 0, 0);
		glColor3f(0, 1, 0);
		glVertex3f(0, 0, 0);
		glVertex3f(0, 1, 0);
		glColor3f(0, 0, 1);
		glVertex3f(0, 0, 0);
		glVertex3f(0, 0, 1);

		glColor3f(0.5, 0.5, 0.5);
		glVertex3f(0, 0, 0);
		glVertex3f(-1, 0, 0);
		glVertex3f(0, 0, 0);
		glVertex3f(0, -1, 0);
		glVertex3f(0, 0, 0);
		glVertex3f(0, 0, -1);

		glEnd();
	}

	glPopMatrix();
}




void display_func(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Draw the model's components using OpenGL/GLUT primitives.
	draw_objects();

	if (true == draw_control_list)
	{
		// Text drawing code originally from "GLUT Tutorial -- Bitmap Fonts and Orthogonal Projections" by A R Fernandes
		// http://www.lighthouse3d.com/opengl/glut/index.php?bmpfontortho
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluOrtho2D(0, win_x, 0, win_y);
		glScalef(1, -1, 1); // Neat. :)
		glTranslatef(0, -win_y, 0); // Neat. :)
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		glColor3f(control_list_colour.x, control_list_colour.y, control_list_colour.z);

		size_t break_size = 22;
		size_t start = 20;
		ostringstream oss;

		render_string(10, start, GLUT_BITMAP_HELVETICA_18, string("Mouse controls:"));
		render_string(10, start + 1 * break_size, GLUT_BITMAP_HELVETICA_18, string("  LMB + drag: Rotate camera"));
		render_string(10, start + 2 * break_size, GLUT_BITMAP_HELVETICA_18, string("  RMB + drag: Zoom camera"));

		render_string(10, start + 4 * break_size, GLUT_BITMAP_HELVETICA_18, string("Keyboard controls:"));
		render_string(10, start + 5 * break_size, GLUT_BITMAP_HELVETICA_18, string("  w: Draw axis"));
		render_string(10, start + 6 * break_size, GLUT_BITMAP_HELVETICA_18, string("  e: Draw text"));


		custom_math::vector_3 eye = main_camera.eye;
		custom_math::vector_3 eye_norm = eye;
		eye_norm.normalize();

		oss.clear();
		oss.str("");
		oss << "Camera position: " << eye.x << ' ' << eye.y << ' ' << eye.z;
		render_string(10, win_y - 2 * break_size, GLUT_BITMAP_HELVETICA_18, oss.str());

		oss.clear();
		oss.str("");
		oss << "Camera position (normalized): " << eye_norm.x << ' ' << eye_norm.y << ' ' << eye_norm.z;
		render_string(10, win_y - break_size, GLUT_BITMAP_HELVETICA_18, oss.str());

		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		// End text drawing code.
	}

	glFlush();
	glutSwapBuffers();
}

void keyboard_func(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w':
	{
		draw_axis = !draw_axis;
		break;
	}
	case 'e':
	{
		draw_control_list = !draw_control_list;
		break;
	}

	default:
		break;
	}
}

void mouse_func(int button, int state, int x, int y)
{
	if (GLUT_LEFT_BUTTON == button)
	{
		if (GLUT_DOWN == state)
			lmb_down = true;
		else
			lmb_down = false;
	}
	else if (GLUT_MIDDLE_BUTTON == button)
	{
		if (GLUT_DOWN == state)
			mmb_down = true;
		else
			mmb_down = false;
	}
	else if (GLUT_RIGHT_BUTTON == button)
	{
		if (GLUT_DOWN == state)
			rmb_down = true;
		else
			rmb_down = false;
	}
}

void motion_func(int x, int y)
{
	int prev_mouse_x = mouse_x;
	int prev_mouse_y = mouse_y;

	mouse_x = x;
	mouse_y = y;

	int mouse_delta_x = mouse_x - prev_mouse_x;
	int mouse_delta_y = prev_mouse_y - mouse_y;

	if (true == lmb_down && (0 != mouse_delta_x || 0 != mouse_delta_y))
	{
		main_camera.u -= static_cast<float>(mouse_delta_y) * u_spacer;
		main_camera.v += static_cast<float>(mouse_delta_x) * v_spacer;
	}
	else if (true == rmb_down && (0 != mouse_delta_y))
	{
		main_camera.w -= static_cast<float>(mouse_delta_y) * w_spacer;

		if (main_camera.w < 1.1f)
			main_camera.w = 1.1f;

	}

	main_camera.Set(); // Calculate new camera vectors.
}

void passive_motion_func(int x, int y)
{
	mouse_x = x;
	mouse_y = y;
}



