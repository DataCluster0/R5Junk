#include <includes.h>

using namespace std::filesystem;

struct VGMeshData
{
	uint64_t pos = 0;
	VGMesh h{};
	std::vector<VTX> vtxlist{};
};

std::vector<VGMeshData> ReadVGMeshData(BinaryIO& Reader, VGHeader& hdr, Vector3 NewPosition, Vector3 NewRot)
{
	VGHeader h = Reader.read<VGHeader>();

	if (h.magic != 0x47567430)
	{
		printf("invalid file magic expected '0tVG', found %x\n", h.magic);
		return {};
	}

	std::vector<VGMeshData> Meshes(h.meshCount);

	uint64_t offset = 0;
	for (auto& mesh : Meshes)
	{
		VGMeshData data{};

		mesh.pos = Reader.tell();
		Reader.seek(h.meshOffset + offset);

		mesh.h = Reader.read<VGMesh>();

		VGMesh& m = mesh.h;

		if (m.flags == 0 || m.vertexCount == 0)
			continue;

		uint64_t vertex_offset = Reader.tell();
		Reader.seek(h.vertexOffset + m.vertexOffset);

		mesh.vtxlist.resize(m.vertexCount);

		for (auto& vtx : mesh.vtxlist)
		{
			vtx = VTX::ReadVTX(Reader, m);

			if (m.flags & VG_POSITION)
				vtx.Pos += NewPosition;
			else if (m.flags & VG_PACKED_POSITION)
			{
				if (NewRot != Vector3(0, 0, 0))
				{
					vtx.PKPos.RotatePosition(NewRot);
					vtx.normal.RotateNormal(NewRot);
				}
					
				if (NewPosition != Vector3(0, 0, 0))
					vtx.PKPos.MovePosition(NewPosition);
			}

			vtx.NewPos = vtx.PKPos.UnpackPosition();
		}

		offset += sizeof(VGMesh);
	}

	Reader.close();

	hdr = h;

	return Meshes;
}

void WriteVGMeshData(BinaryIO& writer, VGHeader& hdr, std::vector<VGMeshData>& Data)
{
	uint64_t offset = 0;

	int i = 0;
	for (auto& mesh : Data)
	{
		writer.seek(hdr.vertexOffset + mesh.h.vertexOffset);

		offset += sizeof(VGMesh);
		i++;

		if (mesh.h.flags == 0 || mesh.h.vertexCount == 0)
			continue;

		printf("%d Found %d Verts\n", i, mesh.h.vertexCount);

		for (auto& vtx : mesh.vtxlist)
		{
			auto& h = mesh.h;

			uint64_t position = writer.tell();

			if (h.flags & VG_POSITION)
				writer.write<Vector3>(vtx.Pos);
			else if (h.flags & VG_PACKED_POSITION)
				writer.write<PackedPosition>(vtx.PKPos);

			if (h.flags & VG_PACKED_WEIGHTS)
				writer.write<PackedVertexWeights>(vtx.PKWeights);

			writer.write<PackedNormal>(vtx.normal);

			// vertex color from vvc
			if (h.flags & VG_VERTEX_COLOR)
				writer.write<VertexColor_t>(vtx.color);

			writer.write<Vector2>(vtx.TexCoord);

			// same vector2 from vvc
			if (h.flags & VG_UV_LAYER)
				writer.write<Vector2>(vtx.UVLayer);
		}
	}

	writer.close();
}

int main(int argc, char* argv[]) {
	if (argc < 2)
	{
		printf("\n==============================================\n");
		printf("$ Usage : VGMover.exe <source> <pos x y z> <rot x y z>\n");
		printf("==============================================\n\n");

		printf("$ Usage       : replace\n");
		printf("# Example     : VGMover.exe ./a.vg 0 10.0 0 0 180 180\n");
		printf("? Description : Moves the vg vertex positions\n\n");

		printf("==============================================\n\n");

		return 0;
	}

	std::string SourceFile = std::string(argv[1]);

	auto sourcefile = std::filesystem::path(SourceFile);
	std::string SourceFileName = sourcefile.filename().string();

	if (!FILE_EXISTS(SourceFile))
	{
		printf("source file '%s' not found\n", SourceFileName.c_str());
		return 0;
	}

	Vector3 NewPos{};

	Vector3 NewRot{};

	// pos
	if (argc >= 5)
	{
		float x = ::atof(std::string(argv[2]).c_str());
		float y = ::atof(std::string(argv[3]).c_str());
		float z = ::atof(std::string(argv[4]).c_str());

		NewPos = { x,y,z };
	}
	else
	{
		printf("!!! Move <X Y Z> not provided !!!\n");
		return 0;
	}

	// rot
	if (argc >= 8)
	{
		float x = ::atof(std::string(argv[5]).c_str());
		float y = ::atof(std::string(argv[6]).c_str());
		float z = ::atof(std::string(argv[7]).c_str());

		NewRot = { x,y,z };
	}


	size_t SourceInputSize = Utils::GetFileSize(SourceFile);
	char* pDataBuf = new char[SourceInputSize];

	BinaryIO SourceReader;
	SourceReader.open(SourceFile, BinaryIOMode::Read);
	SourceReader.getReader()->read(pDataBuf, SourceInputSize);
	SourceReader.seek(0);

	VGHeader h{};

	std::vector<VGMeshData> MeshData = ReadVGMeshData(SourceReader, h, NewPos, NewRot);

	BinaryIO writer;

	std::string outpath = sourcefile.parent_path().string() + "\\output\\";

	std::filesystem::create_directory(outpath);
	writer.open(outpath + SourceFileName, BinaryIOMode::Write);

	writer.getWriter()->write(pDataBuf, SourceInputSize);
	writer.seek(0);

	WriteVGMeshData(writer, h, MeshData);

	//system("pause");
	return EXIT_SUCCESS;
}