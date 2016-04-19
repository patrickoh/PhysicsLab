#include "Common.h"
#include "Point.h"
#include "Line.h"
#include "..\Bounce\Bounce.h"

class DebugDrawer : public b3Draw {

public:

	void DrawPoint(const glm::vec3& position, const b3Color& color) const
	{

	}

	void DrawLine(const glm::vec3& a, const glm::vec3& b, const b3Color& color) const
	{
		/*glColor3f(color.R, color.G, color.B);
		glBegin(GL_LINES);
			glVertex3f(a.x, a.y, a.z);
			glVertex3f(b.x, b.y, b.z);
		glEnd();*/

		glColor3f(color.R, color.G, color.B);
		glPointSize(10.0f);
		glBegin(GL_POINTS);
			glVertex3f(a.x, a.y, a.z);
		glEnd();
	}

	void DrawAABB(const b3AABB& aabb, const b3Color& color) const
	{

	}
};