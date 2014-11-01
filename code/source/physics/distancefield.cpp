#include "distancefield.hpp"
#include "chunk_util.hpp"
#include "collision/baseshape_circle.hpp"
#include "collision/query.hpp"
#include "hardware/glstate.hpp"
#include "global/file.hpp"
#include "util/profiling.hpp"

namespace clover {
namespace physics {

util::DynArray<visual::VertexAttribute> PolyVertex::getAttributes(){
	util::DynArray<visual::VertexAttribute> ret;

	{
		visual::VertexAttribute a;
		a.offset= offsetof(PolyVertex, position);
		a.elemType= hardware::GlState::Type::Real32;
		a.elemCount= 2;
		a.name= "a_position";
		ret.pushBack(a);
	}

	{
		visual::VertexAttribute a;
		a.offset= offsetof(PolyVertex, velocity);
		a.elemType= hardware::GlState::Type::Real32;
		a.elemCount= 2;
		a.name= "a_velocity";
		ret.pushBack(a);
	}

	{
		visual::VertexAttribute a;
		a.offset= offsetof(PolyVertex, rotation);
		a.elemType= hardware::GlState::Type::Real32;
		a.elemCount= 1;
		a.name= "a_rotation";
		ret.pushBack(a);
	}

	{
		visual::VertexAttribute a;
		a.offset= offsetof(PolyVertex, angularVelocity);
		a.elemType= hardware::GlState::Type::Real32;
		a.elemCount= 1;
		a.name= "a_angularVelocity";
		ret.pushBack(a);
	}

	{
		visual::VertexAttribute a;
		a.offset= offsetof(PolyVertex, id);
		a.elemType= hardware::GlState::Type::Uint16;
		a.elemCount= 1;
		a.name= "a_id";
		ret.pushBack(a);
	}

	{
		visual::VertexAttribute a;
		a.offset= offsetof(PolyVertex, chunkId);
		a.elemType= hardware::GlState::Type::Uint8;
		a.elemCount= 1;
		a.name= "a_chunkId";
		ret.pushBack(a);
	}

	{
		visual::VertexAttribute a;
		a.offset= offsetof(PolyVertex, vertexCount);
		a.elemType= hardware::GlState::Type::Uint8;
		a.elemCount= 1;
		a.name= "a_vertexCount";
		ret.pushBack(a);
	}

	for (SizeType i= 0; i < maxVertexCount; ++i){
		visual::VertexAttribute a;
		a.offset= offsetof(PolyVertex, vertices) + 8*i;
		a.elemType= hardware::GlState::Type::Real32;
		a.elemCount= 2;
		a.name= util::Str8::format("a_vertex_%i", (int32)i);
		ret.pushBack(a);
	}

	return ret;
}


util::DynArray<visual::VertexAttribute> DistFieldVertex::getAttributes(){	
	util::DynArray<visual::VertexAttribute> ret;

	{
		visual::VertexAttribute a;
		a.offset= offsetof(DistFieldVertex, position);
		a.elemType= hardware::GlState::Type::Real32;
		a.elemCount= 2;
		a.name= "a_position";
		ret.pushBack(a);
	}

	{
		visual::VertexAttribute a;
		a.offset= offsetof(DistFieldVertex, velocity);
		a.elemType= hardware::GlState::Type::Real32;
		a.elemCount= 2;
		a.name= "a_velocity";
		ret.pushBack(a);
	}

	{
		visual::VertexAttribute a;
		a.offset= offsetof(DistFieldVertex, uv);
		a.elemType= hardware::GlState::Type::Real32;
		a.elemCount= 2;
		a.name= "a_uv";
		ret.pushBack(a);
	}

	{
		visual::VertexAttribute a;
		a.offset= offsetof(DistFieldVertex, id);
		a.elemType= hardware::GlState::Type::Uint32;
		a.elemCount= 1;
		a.name= "a_id";
		ret.pushBack(a);
	}

	{
		visual::VertexAttribute a;
		a.offset= offsetof(DistFieldVertex, chunkId);
		a.elemType= hardware::GlState::Type::Uint32;
		a.elemCount= 1;
		a.name= "a_chunkId";
		ret.pushBack(a);
	}

	return ret;
}

util::DynArray<visual::VertexAttribute> VolumVertex::getAttributes(){
	util::DynArray<visual::VertexAttribute> ret;

	{
		visual::VertexAttribute a;
		a.offset= offsetof(VolumVertex, position);
		a.elemType= hardware::GlState::Type::Real32;
		a.elemCount= 2;
		a.name= "a_position";
		ret.pushBack(a);
	}
	
	{
		visual::VertexAttribute a;
		a.offset= offsetof(VolumVertex, uv);
		a.elemType= hardware::GlState::Type::Real32;
		a.elemCount= 3;
		a.name= "a_uv";
		ret.pushBack(a);
	}

	return ret;
}

DistanceField::DistanceField(uint32 chunk_reso, uint32 chunks)
		: chunkCount(chunks){
	{
		const util::Str8 mesh_shd_path= "fluids/distancefieldmesh";
		meshShader.setSources(
				global::File::readText(mesh_shd_path + ".vert"),
				global::File::readText(mesh_shd_path + ".geom"));
		meshShader.setOutputVaryings({
				"g_position",
				"g_velocity",
				"g_uv",
				"g_id",
				"g_chunkId"
		});
		meshShader.compile<PolyVertex>();
		mesh_chunkSizeLoc= meshShader.getUniformLocation("u_chunkSize");
	}

	{
		const util::Str8 draw_shd_path= "fluids/distancefielddraw";
		drawShader.setSources(
				global::File::readText(draw_shd_path + ".vert"),
				global::File::readText(draw_shd_path + ".geom"),
				global::File::readText(draw_shd_path + ".frag"));
		drawShader.compile<DistFieldVertex>();
		draw_chunkSizeLoc= drawShader.getUniformLocation("u_chunkSize");
		draw_chunkCountLoc= drawShader.getUniformLocation("u_chunkCount");
		draw_chunkInfoSamplerLoc= drawShader.getUniformLocation("u_chunkInfo");
		
		visual::Framebuffer::Cfg fbo_cfg;
		fbo_cfg.resolution= util::Vec2i(chunk_reso);
		fbo_cfg.layers= chunks;
		fbo_cfg.linearInterpolation= true;
		fbo_cfg.alpha= true;
		fbo_cfg.depth= true;
		fbo.create(fbo_cfg);
	}

}

void DistanceField::update(
		uint32 chunk_size,
		const PosToIdMap& pos_to_id,
		hardware::GlState::TexDId chunk_info_tex){
	PROFILE();

	PolyGenResult world= genPolys(chunk_size, pos_to_id);

	visual::VertexArrayObject<PolyVertex> poly_vao(hardware::GlState::Primitive::Point);
	poly_vao.submit(world.polys);

	mesh.setUsage(hardware::GlState::VaoUsage::StreamDraw);
	mesh.reserveVertices(world.distMeshTriCount*3);

	//
	// Generate distance field mesh by transform feedback
	//

	{ PROFILE();

		meshShader.use();
		meshShader.setUniform(mesh_chunkSizeLoc, chunk_size);
		
		/// @todo To hardware::GlState
		glEnable(GL_RASTERIZER_DISCARD);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mesh.getVboDId());
	 
		glBeginTransformFeedback(GL_TRIANGLES);

		poly_vao.draw();

		glEndTransformFeedback();

		glDisable(GL_RASTERIZER_DISCARD);
	}

	//
	// Render generated mesh to distance field tex
	//

	{ PROFILE();

		fbo.bind();
		drawShader.use();
		drawShader.setUniform(draw_chunkSizeLoc, chunk_size);
		drawShader.setUniform(draw_chunkCountLoc, chunkCount);
		drawShader.setTexture(draw_chunkInfoSamplerLoc, chunk_info_tex, 0);

		// No blending - depth testing will take care of priorization
		hardware::gGlState->setBlendFunc(hardware::GlState::BlendFunc{GL_ONE, GL_ZERO});
		hardware::gGlState->setClearColor({0, 0, 0, 0});
	
		/// @todo To hardware::GlState
		glEnable(GL_DEPTH_TEST);

		hardware::gGlState->clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		mesh.draw();

		/// @todo To hardware::GlState
		glDisable(GL_DEPTH_TEST);
	}

	//
	// Generate mapped field mesh (this is independent of other calculations)
	//

	{ PROFILE();
		util::DynArray<VolumVertex> vertices;

		for (auto& pair : pos_to_id){
			util::Vec2i pos= pair.first;
			uint8 id= pair.second;

			real32 width= chunk_size;
			util::Vec2f world_pos= pos.casted<util::Vec2f>()*width;
			real32 uv_z= (id + 0.5f)/chunkCount;

			VolumVertex quad[4]= {
				{	world_pos + util::Vec2f{0.0f, 0.0f},
					util::Vec3f{0.0f, 0.0f, uv_z}
				},
				{	world_pos + util::Vec2f{width, 0.0f},
					util::Vec3f{1.0f, 0.0f, uv_z}
				},
				{	world_pos + util::Vec2f{width, width},
					util::Vec3f{1.0f, 1.0f, uv_z}
				},
				{	world_pos + util::Vec2f{0.0f, width},
					util::Vec3f{0.0f, 1.0f, uv_z}
				}
			};
			
			vertices.pushBack(quad[0]);
			vertices.pushBack(quad[1]);
			vertices.pushBack(quad[2]);
			
			vertices.pushBack(quad[0]);
			vertices.pushBack(quad[2]);
			vertices.pushBack(quad[3]);
		}

		mappedFieldMesh.submit(vertices);
	}

}

auto DistanceField::genPolys(
		uint32 chunk_size,
		const PosToIdMap& pos_to_id) const -> PolyGenResult {
	PROFILE();

	util::DynArray<const b2Shape*, game::SingleFrameAtor> shapes;
	util::DynArray<PolyVertex, game::SingleFrameAtor> polygons;

	// Triangle count in extruded distance mesh
	SizeType distmesh_triangle_count= 0;

	{ PROFILE();
		SizeType estimated_max_shape_count=
			RigidObject::pooledCount()*2;

		shapes.reserve(estimated_max_shape_count);
		polygons.reserve(estimated_max_shape_count);
	}

	// Fill in object data
	const auto& pool= RigidObject::poolView();
	for (SizeType obj_i= 0; obj_i < pool.size(); ++obj_i){
		RigidObject* obj;

		{ PROFILE();
			obj= pool[obj_i];
		}

		{ PROFILE();
			if (!obj || !obj->isActive())
				continue;
		}

		PolyVertex poly;
		{ PROFILE();
			// Expensive
			poly.position= asChunkOffset(obj->getPosition(), chunk_size);
			poly.velocity= obj->getPosition().casted<util::Vec2f>();
			poly.rotation= (real32)obj->getRotation();
			poly.angularVelocity= (real32)obj->getAngularVelocity();
			poly.id= static_cast<uint16>(obj_i);
		}

		{ PROFILE();
			util::Vec2i chunk_pos= chunkVec(obj->getPosition(), chunk_size);
			auto chunk_it= pos_to_id.find(chunk_pos);
			if (chunk_it == pos_to_id.end()){
				continue;
			}
			else {
				poly.chunkId= chunk_it->second;
			}
		}

		/// @note	This can be optimized by caching shape
		///			pointers in RigidObject
		{
			for (auto& fix : obj->getRigidFixtures()){
				const collision::Shape* shp= fix->getShape();
				if (!shp)
					continue;
				for (auto& base_shp : shp->getBaseShapes()){
					for (SizeType i= 0; i < base_shp->getB2ShapeCount(); ++i){
						shapes.pushBack(&base_shp->getB2Shape(i));
						polygons.pushBack(poly);
					}
				}
			}
		}
	}

	// Fill in polygon data
	{ PROFILE();
		ensure(polygons.size() == shapes.size());
		for (SizeType i= 0; i < polygons.size(); ++i){
			PolyVertex& poly= polygons[i];
			const b2Shape& b2_shp= *NONULL(shapes[i]);

			b2Shape::Type type= b2_shp.GetType();
			if (type == b2Shape::e_polygon){
				auto b2_poly= *NONULL(static_cast<const b2PolygonShape*>(&b2_shp));
				
				ensure(b2_poly.m_count <= (int32)PolyVertex::maxVertexCount);
				for (int32 i= 0; i < b2_poly.m_count; ++i){
					ensure(i < (int32)PolyVertex::maxVertexCount);
					poly.vertices[i]= util::Vec2f(b2_poly.m_vertices[i]);
				}
				poly.vertexCount= b2_poly.m_count;
			}
			else if (type == b2Shape::e_circle){
				auto b2_circle=
					*NONULL(static_cast<const b2CircleShape*>(&b2_shp));

				/// @todo Adjust vertex count by size
				util::DynArray<util::Vec2d> verts=
					collision::circleVertices(
							b2_circle.m_p,
							b2_circle.m_radius,
							PolyVertex::maxVertexCount);
				ensure(verts.size() <= PolyVertex::maxVertexCount);

				for (SizeType i= 0; i < verts.size(); ++i){
					poly.vertices[i]= verts[i].casted<util::Vec2f>();
				}
				poly.vertexCount= verts.size();
			}
			else {
				continue;
			}
		
			distmesh_triangle_count +=
				(poly.vertexCount - 2) + // Inner polygon
				poly.vertexCount*2 + // Extruded edges
				poly.vertexCount*2; // Corner triangles
		}
	}

	PolyGenResult result;
	result.polys= std::move(polygons);
	result.distMeshTriCount= distmesh_triangle_count;
	return result;
}

} // physics
} // clover
